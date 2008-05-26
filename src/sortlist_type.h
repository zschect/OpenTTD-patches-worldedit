/* $Id$ */

/** @file sortlist_type.h Base types for having sorted lists in GUIs. */

#ifndef SORTLIST_TYPE_H
#define SORTLIST_TYPE_H

#include "misc/smallvec.h"
#include "date_type.h"

enum SortListFlags {
	VL_NONE       = 0,      ///< no sort
	VL_DESC       = 1 << 0, ///< sort descending or ascending
	VL_RESORT     = 1 << 1, ///< instruct the code to resort the list in the next loop
	VL_REBUILD    = 1 << 2, ///< rebuild the sort list
	VL_FIRST_SORT = 1 << 3, ///< sort with qsort first
	VL_END        = 1 << 4,
};
DECLARE_ENUM_AS_BIT_SET(SortListFlags);

struct Listing {
	bool order;    ///< Ascending/descending
	byte criteria; ///< Sorting criteria
};

template <typename T>
class GUIList : public SmallVector<T, 32> {
public:
	typedef int SortFunction(const T*, const T*);

public: // Temporary: public for conversion only
	SortFunction* const *func_list; ///< The sort criteria functions
	SortListFlags flags;            ///< used to control sorting/resorting/etc.
	uint8 sort_type;                ///< what criteria to sort on
	uint16 resort_timer;            ///< resort list after a given amount of ticks if set

	/**
	 * Check if the list is sortable
	 *
	 * @return true if we can sort the list
	 */
	bool IsSortable() const
	{
		return (this->data != NULL && this->items >= 2);
	}

	/**
	 * Reset the resort timer
	 */
	void ResetResortTimer()
	{
		/* Resort every 10 days */
		this->resort_timer = DAY_TICKS * 10;
	}

	/**
	 * Reverse the list
	 */
	void Reverse()
	{
		assert(this->IsSortable());

		T *a = this->data;
		T *b = a + (this->items - 1);

		do {
			Swap(*a, *b);
		} while (++a < --b);
	}

public:
	GUIList() :
		func_list(NULL),
		flags(VL_FIRST_SORT),
		sort_type(0),
		resort_timer(1)
	{};

	/**
	 * Get the sorttype of the list
	 *
	 * @return The current sorttype
	 */
	uint8 SortType() const
	{
		return this->sort_type;
	}

	/**
	 * Set the sorttype of the list
	 *
	 * @param n_type the new sort type
	 */
	void SetSortType(uint8 n_type)
	{
		if (this->sort_type != n_type) {
			SETBITS(this->flags, VL_RESORT | VL_FIRST_SORT);
			this->sort_type = n_type;
		}
	}

	/**
	 * Export current sort conditions
	 *
	 * @return the current sort conditions
	 */
	Listing GetListing() const
	{
		Listing l;
		l.order = HASBITS(this->flags, VL_DESC);
		l.criteria = this->sort_type;

		return l;
	}

	/**
	 * Import sort conditions
	 *
	 * @param l The sport conditions we want to use
	 */
	void SetListing(Listing l)
	{
		if (l.order) {
			SETBITS(this->flags, VL_DESC);
		} else {
			CLRBITS(this->flags, VL_DESC);
		}
		this->sort_type = l.criteria;

		SETBITS(this->flags, VL_FIRST_SORT);
	}

	/**
	 * Check if a resort is needed next loop
	 *  If used the resort timer will decrease every call
	 *  till 0. If 0 reached the resort bit will be set and
	 *  the timer will be reset.
	 *
	 * @return true if resort bit is set for next loop
	 */
	bool NeedResort()
	{
		if (--this->resort_timer == 0) {
			SETBITS(this->flags, VL_RESORT);
			this->ResetResortTimer();
			return true;
		}
		return false;
	}

	/**
	 * Force a resort next Sort call
	 *  Reset the resort timer if used too.
	 */
	void ForceResort()
	{
		SETBITS(this->flags, VL_RESORT);
	}

	/**
	 * Check if the sort order is descending
	 *
	 * @return true if the sort order is descending
	 */
	bool IsDescSortOrder() const
	{
		return HASBITS(this->flags, VL_DESC);
	}

	/**
	 * Toogle the sort order
	 *  Since that is the worst condition for the sort function
	 *  reverse the list here.
	 */
	FORCEINLINE void ToggleSortOrder()
	{
		this->flags ^= VL_DESC;

		if (this->IsSortable()) this->Reverse();
	}

	/**
	 * GnomeSort algorithm
	 *  This sorting uses a slightly modifyied Gnome search.
	 *  The basic Gnome search trys to sort already sorted
	 *  list parts. The modification skips these. For the first
	 *  sorting we use qsort since it is faster for irregular
	 *  sorted data.
	 *
	 * @param compare The function to compare two list items
	 * */
	FORCEINLINE void Sort(SortFunction compare)
	{
		/* Do not sort if the resort bit is not set */
		if (!HASBITS(this->flags, VL_RESORT)) return;

		CLRBITS(this->flags, VL_RESORT);

		this->ResetResortTimer();

		/* Do not sort when the list is not sortable */
		if (!this->IsSortable()) return;

		const bool desc = HASBITS(this->flags, VL_DESC);

		if (HASBITS(this->flags, VL_FIRST_SORT)) {
			qsort(this->data, this->items, sizeof(T), (int (*)(const void *a, const void *b))compare);

			if (desc) this->Reverse();
			return;
		}

		T *a = this->data;
		T *b = a + 1;

		uint length = this->items;
		uint offset = 0; // Jump variable

		while (length > 1) {
			const int diff = compare(a, b);
			if ((!desc && diff <= 0) || (desc && diff >= 0)) {
				if (offset != 0) {
					/* Jump back to the last direction switch point */
					a += offset;
					b += offset;
					offset = 0;
					continue;
				}
				a++;
				b++;
				length--;
			} else {
				Swap(*a, *b);
				if (a != this->data) {
					offset++;
					a--;
					b--;
				}
			}
		}
	}

	/**
	 * Hand the array of sort function pointers to the sort list
	 *
	 * @param n_funcs The pointer to the first sort func
	 */
	void SetSortFuncs(SortFunction* const* n_funcs)
	{
		this->func_list = n_funcs;
	}

	/**
	 * Overload of Sort()
	 * Overloaded to reduce external code
	 */
	void Sort()
	{
		assert(this->func_list != NULL);
		this->Sort(this->func_list[this->sort_type]);
	}

	/**
	 * Check if a rebuild is needed
	 * @return true if a rebuild is needed
	 */
	bool NeedRebuild() const
	{
		return HASBITS(this->flags, VL_REBUILD);
	}

	/**
	 * Force that a rebuild is needed
	 */
	void ForceRebuild()
	{
		SETBITS(this->flags, VL_REBUILD);
	}

	/**
	 * Notify the sortlist that the rebuild is done
	 *
	 * @note This forces a resort
	 */
	void RebuildDone()
	{
		CLRBITS(this->flags, VL_REBUILD);
		SETBITS(this->flags, VL_RESORT);
	}
};

#endif /* SORTLIST_TYPE_H */
