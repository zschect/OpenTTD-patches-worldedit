/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file script_scanner.cpp Allows scanning for scripts. */

#include "../stdafx.h"
#include "../debug.h"
#include "../string_func.h"
#include "../settings_type.h"

#include "../script/squirrel.hpp"
#include "script_scanner.hpp"
#include "script_info.hpp"
#include "script_fatalerror.hpp"

#include "../network/network_content.h"
#include "../3rdparty/md5/md5.h"
#include "../tar_type.h"
#include "../3rdparty/fmt/format.h"

#include "../safeguards.h"

bool ScriptScanner::AddFile(const std::string &filename, size_t basepath_length, const std::string &tar_filename)
{
	this->main_script = filename;
	this->tar_file = tar_filename;

	auto p = this->main_script.rfind(PATHSEPCHAR);
	this->main_script.erase(p != std::string::npos ? p + 1 : 0);
	this->main_script += "main.nut";

	if (!FioCheckFileExists(filename, this->subdir) || !FioCheckFileExists(this->main_script, this->subdir)) return false;

	this->ResetEngine();
	try {
		this->engine->LoadScript(filename.c_str());
	} catch (Script_FatalError &e) {
		DEBUG(script, 0, "Fatal error '%s' when trying to load the script '%s'.", e.GetErrorMessage().c_str(), filename.c_str());
		return false;
	}
	return true;
}

ScriptScanner::ScriptScanner() :
	engine(nullptr)
{
}

void ScriptScanner::ResetEngine()
{
	this->engine->Reset();
	this->engine->SetGlobalPointer(this);
	this->RegisterAPI(this->engine);
}

void ScriptScanner::Initialize(const char *name)
{
	this->engine = new Squirrel(name);

	this->RescanDir();

	this->ResetEngine();
}

ScriptScanner::~ScriptScanner()
{
	this->Reset();

	delete this->engine;
}

void ScriptScanner::RescanDir()
{
	/* Forget about older scans */
	this->Reset();

	/* Scan for scripts */
	this->Scan(this->GetFileName(), this->GetDirectory());
}

void ScriptScanner::Reset()
{
	for (const auto &item : this->info_list) {
		delete item.second;
	}

	this->info_list.clear();
	this->info_single_list.clear();
}

void ScriptScanner::RegisterScript(ScriptInfo *info)
{
	std::string script_original_name = this->GetScriptName(info);
	std::string script_name = fmt::format("{}.{}", script_original_name, info->GetVersion());

	/* Check if GetShortName follows the rules */
	if (strlen(info->GetShortName()) != 4) {
		DEBUG(script, 0, "The script '%s' returned a string from GetShortName() which is not four characaters. Unable to load the script.", info->GetName());
		delete info;
		return;
	}

	if (this->info_list.find(script_name) != this->info_list.end()) {
		/* This script was already registered */
#ifdef _WIN32
		/* Windows doesn't care about the case */
		if (StrEqualsIgnoreCase(this->info_list[script_name]->GetMainScript(), info->GetMainScript()) == 0) {
#else
		if (strcmp(this->info_list[script_name]->GetMainScript(), info->GetMainScript()) == 0) {
#endif
			delete info;
			return;
		}

		DEBUG(script, 1, "Registering two scripts with the same name and version");
		DEBUG(script, 1, "  1: %s", this->info_list[script_name]->GetMainScript());
		DEBUG(script, 1, "  2: %s", info->GetMainScript());
		DEBUG(script, 1, "The first is taking precedence.");

		delete info;
		return;
	}

	this->info_list[script_name] = info;

	if (!info->IsDeveloperOnly() || _settings_client.gui.ai_developer_tools) {
		/* Add the script to the 'unique' script list, where only the highest version
		 *  of the script is registered. */
		auto it = this->info_single_list.find(script_original_name);
		if (it == this->info_single_list.end()) {
			this->info_single_list[script_original_name] = info;
		} else if (it->second->GetVersion() < info->GetVersion()) {
			it->second = info;
		}
	}
}

std::string ScriptScanner::GetConsoleList(bool newest_only) const
{
	std::string p;
	p += stdstr_fmt("List of %s:\n", this->GetScannerName());
	const ScriptInfoList &list = newest_only ? this->info_single_list : this->info_list;
	for (const auto &item : list) {
		ScriptInfo *i = item.second;
		p += stdstr_fmt("%10s (v%d): %s\n", i->GetName(), i->GetVersion(), i->GetDescription());
	}
	p += "\n";

	return p;
}

/** Helper for creating a MD5sum of all files within of a script. */
struct ScriptFileChecksumCreator : FileScanner {
	byte md5sum[16];  ///< The final md5sum.
	Subdirectory dir; ///< The directory to look in.

	/**
	 * Initialise the md5sum to be all zeroes,
	 * so we can easily xor the data.
	 */
	ScriptFileChecksumCreator(Subdirectory dir)
	{
		this->dir = dir;
		memset(this->md5sum, 0, sizeof(this->md5sum));
	}

	/* Add the file and calculate the md5 sum. */
	virtual bool AddFile(const std::string &filename, size_t basepath_length, const std::string &tar_filename)
	{
		Md5 checksum;
		uint8 buffer[1024];
		size_t len, size;
		byte tmp_md5sum[16];

		/* Open the file ... */
		FILE *f = FioFOpenFile(filename.c_str(), "rb", this->dir, &size);
		if (f == nullptr) return false;

		/* ... calculate md5sum... */
		while ((len = fread(buffer, 1, (size > sizeof(buffer)) ? sizeof(buffer) : size, f)) != 0 && size != 0) {
			size -= len;
			checksum.Append(buffer, len);
		}
		checksum.Finish(tmp_md5sum);

		FioFCloseFile(f);

		/* ... and xor it to the overall md5sum. */
		for (uint i = 0; i < sizeof(md5sum); i++) this->md5sum[i] ^= tmp_md5sum[i];

		return true;
	}
};

/**
 * Check whether the script given in info is the same as in ci based
 * on the shortname and md5 sum.
 * @param ci The information to compare to.
 * @param md5sum Whether to check the MD5 checksum.
 * @param info The script to get the shortname and md5 sum from.
 * @return True iff they're the same.
 */
static bool IsSameScript(const ContentInfo *ci, bool md5sum, ScriptInfo *info, Subdirectory dir)
{
	uint32 id = 0;
	const char *str = info->GetShortName();
	for (int j = 0; j < 4 && *str != '\0'; j++, str++) id |= *str << (8 * j);

	if (id != ci->unique_id) return false;
	if (!md5sum) return true;

	ScriptFileChecksumCreator checksum(dir);
	auto tar_filename = info->GetTarFile();
	TarList::iterator iter;
	if (!tar_filename.empty() && (iter = _tar_list[dir].find(tar_filename)) != _tar_list[dir].end()) {
		/* The main script is in a tar file, so find all files that
		 * are in the same tar and add them to the MD5 checksumming. */
		for (const auto &tar : _tar_filelist[dir]) {
			/* Not in the same tar. */
			if (tar.second.tar_filename != iter->first) continue;

			/* Check the extension. */
			const char *ext = strrchr(tar.first.c_str(), '.');
			if (ext == nullptr || !StrEqualsIgnoreCase(ext, ".nut")) continue;

			checksum.AddFile(tar.first, 0, tar_filename);
		}
	} else {
		char path[MAX_PATH];
		strecpy(path, info->GetMainScript(), lastof(path));
		/* There'll always be at least 1 path separator character in a script
		 * main script name as the search algorithm requires the main script to
		 * be in a subdirectory of the script directory; so <dir>/<path>/main.nut. */
		*strrchr(path, PATHSEPCHAR) = '\0';
		checksum.Scan(".nut", path);
	}

	return memcmp(ci->md5sum, checksum.md5sum, sizeof(ci->md5sum)) == 0;
}

bool ScriptScanner::HasScript(const ContentInfo *ci, bool md5sum)
{
	for (const auto &item : this->info_list) {
		if (IsSameScript(ci, md5sum, item.second, this->GetDirectory())) return true;
	}
	return false;
}

const char *ScriptScanner::FindMainScript(const ContentInfo *ci, bool md5sum)
{
	for (const auto &item : this->info_list) {
		if (IsSameScript(ci, md5sum, item.second, this->GetDirectory())) return item.second->GetMainScript();
	}
	return nullptr;
}
