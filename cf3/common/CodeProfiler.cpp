// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CodeProfiler.hpp"

namespace cf3 {
namespace common {

CodeProfiler::CodeProfiler(const std::string& name) : Component (name)
{
   
}

CodeProfiler::~CodeProfiler()
{
}

} // common
} // cf3

