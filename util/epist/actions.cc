// -*- mode: C++; indent-tabs-mode: nil; -*-
// actions.cc for Epistophy - a key handler for NETWM/EWMH window managers.
// Copyright (c) 2002 - 2002 Ben Jansens <ben at orodu.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "actions.hh"

Action::Action(enum ActionType type, KeyCode keycode, unsigned int modifierMask,
               const std::string &str)
  : _type(type), _keycode(keycode), _modifierMask(modifierMask)
{
  // These are the action types that take string arguments. This
  // should probably be moved to a static member
  ActionType str_types[] = {
    execute,
    nextWindowOfClass,
    prevWindowOfClass,
    nextWindowOfClassOnAllWorkspaces,
    prevWindowOfClassOnAllWorkspaces,
    noaction
  };

  for (int i = 0; str_types[i] != noaction; ++i) {
    if (type == str_types[i]) {
      _stringParam = str;
      return;
    }
  }
  
  _numberParam = atoi( str.c_str() );

  if (type == changeWorkspace)
    _numberParam;
}
