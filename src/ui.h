/*
# Copyright (C) 2015 Fulvio Benini

* This file is part of Scid (Shane's Chess Information Database).
*
* Scid is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation.
*
* Scid is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Scid. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SCID_UI__H
#define SCID_UI__H

#include "misc.h"
#ifndef CHECKUIDEP
#include "ui_tcltk.h"
#else
//Dummy functions useful to catch unwanted dependencies
namespace UI_impl {

typedef int   UI_res_t;
typedef void* UI_extra_t;
typedef void* UI_handle_t;

inline int Main (int argc, char* argv[], void (*exit) (void*)) {
	return 0;
}
inline Progress CreateProgress(UI_handle_t) {
	return Progress();
}
inline Progress CreateProgressPosMask(UI_handle_t) {
	return Progress();
}
class List {
public:
	explicit List(size_t) {}
	void clear() {}
	template <typename T> void push_back(const T&) {}
};
inline UI_res_t Result(UI_handle_t, errorT) {
	return 0;
}
template <typename T>
inline UI_res_t Result(UI_handle_t, errorT, const T&) {
	return 0;
}

}
#endif


/*
 * Interface for communication between UI and c++ server.
 *
 * The interaction between the UI and c++ code is of type client/server.
 * UI calls sc_ functions, the c++ server execute the operation and returns
 * a success/error code plus, optionally, some results.
 *
 * The only server-side event generated by c++ code is for long operations:
 * it will repeatedly report to UI the amount of work done until completion.
 * UI should respond to this events with true (continue) or false (interrupt).
 */


using UI_impl::UI_res_t;
using UI_impl::UI_extra_t;
using UI_impl::UI_handle_t;

/**
 * sc_*() - Execute server side operations
 * Each function usually have subcommands.
 * See functions implemenations for more usage info.
 */
UI_res_t str_is_prefix  (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t str_prefix_len (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_base        (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_book        (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_clipbase    (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_eco         (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_filter      (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_game        (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_info        (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_move        (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_name        (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_report      (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_pos         (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_search      (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_tree        (UI_extra_t, UI_handle_t, int argc, const char ** argv);
UI_res_t sc_var         (UI_extra_t, UI_handle_t, int argc, const char ** argv);


/**
 * UI_Main() - Init the UI
 * @exit:      clean up function to be called when closing UI
 */
inline int UI_Main (int argc, char* argv[], void (*exit) (void*)) {
	return UI_impl::Main(argc, argv, exit);
}


/**
 * UI_CreateProgress() - create a Progress object
 *
 * With this function c++ code contact the UI to report that the operation
 * asked may take a long time.
 * Then c++ code call Progress::report repeatedly to inform the UI about
 * the percentage of work done and an estimated time to complete the operation.
 * Progress::report will return false if the UI wants to interrupt the operation
 *
 * Return:
 * a Progress object that represent the server->UI async communication, or
 * an empty Progress() if the UI is not interested in the progress report.
 */
inline Progress UI_CreateProgress(UI_handle_t ti) {
	return UI_impl::CreateProgress(ti);
}
inline Progress UI_CreateProgressPosMask(UI_handle_t ti) {
	return UI_impl::CreateProgressPosMask(ti);
}


/**
 * UI_Result() - pass the result of an operation from c++ to UI
 * @res:   OK for success or an error code (error.h)
 * @value: a value (or a list of values, see UI_List) to pass to UI
 *
 * Typical usage:
 * UI_Result(ti, OK);
 * UI_Result(ti, OK, "string value");
 * UI_Result(ti, OK, 5);
 */
inline UI_res_t UI_Result(UI_handle_t ti, errorT res) {
	return UI_impl::Result(ti, res);
}
template <typename T>
inline UI_res_t UI_Result(UI_handle_t ti, errorT res, const T& value) {
	return UI_impl::Result(ti, res, value);
}


/**
 * class UI_List - create a list of values to be sent to UI
 * @max_size:   currently there is no automatic reallocation in push_back()
 *              so the constructor must know the max number of values that
 *              will be stored in the list
 *
 * An heterogeneous container used to pass values from c++ to UI
 *
 * Typical usage:
 * UI_List uiList(2);
 * uiList.push_back("string value");
 * uiList.push_back(5);
 * UI_Result(ti, OK, uiList)
 */
class UI_List : public UI_impl::List {
public:
	explicit UI_List(size_t max_size)
	: UI_impl::List(max_size) {
	}

	/**
	 * Inherited from UI_impl::List
	 *
	void clear();
	template <typename T> void push_back(const T& value);
	 */
};


#endif