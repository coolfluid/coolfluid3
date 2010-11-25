// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CodeProfiler.hpp"

namespace CF {
namespace Common {

CodeProfiler::CodeProfiler(const std::string& name) : Component (name)
{
  BuildComponent<full>().build(this);
}

CodeProfiler::~CodeProfiler()
{
}

} // Common
} // CF

