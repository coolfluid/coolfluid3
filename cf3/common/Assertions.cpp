// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>
#include <iostream>

#include "common/CF.hpp"
#include "common/Log.hpp"
#include "common/PE/Comm.hpp"

#include "common/BasicExceptions.hpp"
#include "common/OSystemLayer.hpp"

#include "common/OSystem.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

AssertionManager::AssertionManager() :
  DoAssertions    ( true ),
  AssertionDumps  ( true ),
  AssertionThrows ( true ) {}

////////////////////////////////////////////////////////////////////////////////

AssertionManager& AssertionManager::instance()
{
  static AssertionManager assertion_manager;
  return assertion_manager;
}

////////////////////////////////////////////////////////////////////////////////

void AssertionManager::do_assert ( bool condition,
                                   const char * cond_str,
                                   const char * file,
                                   int line,
                                   const char * func,
                                   const char * desc )
{
  if ( (!condition) && AssertionManager::instance().DoAssertions )
  {
    CodeLocation code_position (file,line,func);

    std::ostringstream oss;
    oss << "Assertion failed on rank " << PE::Comm::instance().rank() << ": [" << cond_str << "] ";

    if (desc)
      oss << "'" << desc << "' ";

    oss << "at " << code_position.str();

    if ( AssertionManager::instance().AssertionDumps )
      oss << "\n" << OSystem::instance().layer()->back_trace();

    if ( AssertionManager::instance().AssertionThrows )
    {
      throw FailedAssertion ( code_position, oss.str());
    }
    else
    {
      CFerror.setFilterRankZero(false);
      CFerror << oss.str() << CFendl;
      CFerror << "aborting..." << CFendl;
      abort ();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
