// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __python_hh
#define   __python_hh

/*! @file python.hh
  @brief wee
*/

#include "actions.hh"
#include "widget.hh"
#include "bindings.hh"

extern "C" {
#include <Python.h>
}

namespace ob {

//! Add a python callback funtion to the back of the hook list
/*!
  Registering functions for KeyPress events is pointless. Use python_bind_key
  instead to do this.
*/
bool python_register(int action, PyObject *callback);
//! Add a python callback funtion to the front of the hook list
/*!
  Registering functions for KeyPress events is pointless. Use python_bind_key
  instead to do this.
*/
bool python_preregister(int action, PyObject *callback);
//! Remove a python callback function from the hook list
bool python_unregister(int action, PyObject *callback);

//! Removes all python callback functions from the hook list
bool python_unregister_all(int action);

//! Add a keybinding
/*!
  @param keylist A python list of modifier/key/buttons, in the form:
                 "C-A-space" or "A-Button1" etc.
  @param callback A python function to call when the binding is used.
*/
bool python_bind_key(PyObject *keylist, PyObject *callback);

bool python_unbind_key(PyObject *keylist);

void python_set_reset_key(const std::string &key);

//! Adds a mouse binding
/*!
  Bindings do not generate motion events. You can only handle motion events by
  using register to set a function for all motion events. Bindings do generate
  ButtonPress, ButtonRelease, Click, and DoubleClick events.
*/
bool python_bind_mouse(const std::string &button, PyObject *callback);

bool python_unbind_mouse(const std::string &button);

void python_unbind_all();

//! Fire a python callback function
void python_callback(OBActions::ActionType action, Window window,
                     OBWidget::WidgetType type, unsigned int state,
                     long d1 = LONG_MIN, long d2 = LONG_MIN,
                     long d3 = LONG_MIN, long d4 = LONG_MIN);

void python_callback_binding(int id, OBActions::ActionType action,
                             Window window, unsigned int state,
                             unsigned int keybutton, Time time);

}

#endif // __python_hh
