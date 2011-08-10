// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>
#include <iostream>

#include "Common/CF.hpp"
#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"

#include "Common/BasicExceptions.hpp"
#include "Common/OSystemLayer.hpp"

#include "Common/OSystem.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

AssertionManager::AssertionManager() :
  DoAssertions    ( true ),
  AssertionDumps  ( true ),
  AssertionThrows ( false ) {}

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
    oss << "Assertion failed on rank " << Comm::PE::instance().rank() << ": [" << cond_str << "] ";

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
      CFerror << oss.str() << CFendl;
      CFerror << "aborting..." << CFendl;
      abort ();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
