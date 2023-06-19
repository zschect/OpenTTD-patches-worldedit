/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file town_widget.h Types related to the town widgets. */

#ifndef WIDGETS_TOWN_WIDGET_H
#define WIDGETS_TOWN_WIDGET_H

/** Widgets of the #TownDirectoryWindow class. */
enum TownDirectoryWidgets {
	WID_TD_SORT_ORDER,       ///< Direction of sort dropdown.
	WID_TD_SORT_CRITERIA,    ///< Criteria of sort dropdown.
	WID_TD_FILTER,           ///< Filter of name.
	WID_TD_LIST,             ///< List of towns.
	WID_TD_SCROLLBAR,        ///< Scrollbar for the town list.
	WID_TD_WORLD_POPULATION, ///< The world's population.
};

/** Widgets of the #TownAuthorityWindow class. */
enum TownAuthorityWidgets {
	WID_TA_CAPTION,      ///< Caption of window.
	WID_TA_ZONE_BUTTON,  ///< Turn on/off showing local authority zone.
	WID_TA_RATING_INFO,  ///< Overview with ratings for each company.
	WID_TA_COMMAND_LIST, ///< List of commands for the player.
	WID_TA_SCROLLBAR,    ///< Scrollbar of the list of commands.
	WID_TA_ACTION_INFO,  ///< Additional information about the action.
	WID_TA_EXECUTE,      ///< Do-it button.
	WID_TA_SETTING,      ///< Setting drop-down.
	WID_TA_BTN_SEL,      ///< Button selector.
};

/** Widgets of the #TownViewWindow class. */
enum TownViewWidgets {
	WID_TV_CAPTION,        ///< Caption of window.
	WID_TV_VIEWPORT,       ///< View of the center of the town.
	WID_TV_INFO,           ///< General information about the town.
	WID_TV_CENTER_VIEW,    ///< Center the main view on this town.
	WID_TV_SHOW_AUTHORITY, ///< Show the town authority window.
	WID_TV_CHANGE_NAME,    ///< Change the name of this town.
	WID_TV_CATCHMENT,      ///< Toggle catchment area highlight.
	WID_TV_EXPAND,         ///< Expand this town (scenario editor only).
	WID_TV_DELETE,         ///< Delete this town (scenario editor only).
};

/** Widgets of the #FoundTownWindow class. */
enum TownFoundingWidgets {
	WID_TF_NEW_TOWN,          ///< Create a new town.
	WID_TF_RANDOM_TOWN,       ///< Randomly place a town.
	WID_TF_MANY_RANDOM_TOWNS, ///< Randomly place many towns.
	WID_TF_MANY_TOWNS_CMV,    ///< Place many towns from a CMV file.
	WID_TF_EXPAND_ALL_TOWNS,  ///< Make all towns grow slightly.
	WID_TF_TOWN_NAME_EDITBOX, ///< Editor for the town name.
	WID_TF_TOWN_NAME_RANDOM,  ///< Generate a random town name.
	WID_TF_SIZE_SMALL,        ///< Selection for a small town.
	WID_TF_SIZE_MEDIUM,       ///< Selection for a medium town.
	WID_TF_SIZE_LARGE,        ///< Selection for a large town.
	WID_TF_SIZE_RANDOM,       ///< Selection for a randomly sized town.
	WID_TF_CITY,              ///< Selection for the town's city state.
	WID_TF_LAYOUT_ORIGINAL,   ///< Selection for the original town layout.
	WID_TF_LAYOUT_BETTER,     ///< Selection for the better town layout.
	WID_TF_LAYOUT_GRID2,      ///< Selection for the 2x2 grid town layout.
	WID_TF_LAYOUT_GRID3,      ///< Selection for the 3x3 grid town layout.
	WID_TF_LAYOUT_RANDOM,     ///< Selection for a randomly chosen town layout.
};

/** Widgets of the #HousePickerWindow class. */
enum HousePickerWidgets {
	WID_HP_CAPTION,
	WID_HP_MAIN_PANEL_SEL,      ///< Selection widget to show/hide the main panel.
	WID_HP_HOUSE_SETS_SEL,      ///< Selection widget to show/hide the list of house sets.
	WID_HP_HOUSE_SETS,          ///< List of available house sets.
	WID_HP_HOUSE_SELECT_MATRIX, ///< Matrix with houses to select.
	WID_HP_HOUSE_SELECT_SCROLL, ///< Scrollbar associated with the house matrix.
	WID_HP_HOUSE_SELECT,        ///< Panels with house images in the house matrix.
	WID_HP_HOUSE_PREVIEW,       ///< House preview panel.
	WID_HP_HOUSE_NAME,          ///< House name display.
	WID_HP_HISTORICAL_BUILDING, ///< "Historical building" label.
	WID_HP_HOUSE_POPULATION,    ///< House population display.
	WID_HP_HOUSE_ZONES,         ///< House zones display.
	WID_HP_HOUSE_LANDSCAPE,     ///< Information about house availability against the landscape.
	WID_HP_HOUSE_LANDSCAPE_SEL, ///< Selection widget to show/hide the landscape info.
	WID_HP_HOUSE_YEARS,         ///< Years display.
	WID_HP_HOUSE_ACCEPTANCE,    ///< Cargo accepted.
	WID_HP_HOUSE_SUPPLY,        ///< Cargo supplied.
};

/** Widgets of the #SelectTownWindow class. */
enum SelectTownWidgets {
	WID_ST_CAPTION,   ///< Caption of the window.
	WID_ST_PANEL,     ///< Main panel.
	WID_ST_SCROLLBAR, ///< Scrollbar of the panel.
};

#endif /* WIDGETS_TOWN_WIDGET_H */
