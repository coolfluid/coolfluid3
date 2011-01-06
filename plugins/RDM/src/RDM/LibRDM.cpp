// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/ScalarAdvection.hpp"

namespace CF {
namespace RDM {

using namespace CF::Common;

CF::Common::RegistLibrary<LibRDM> libRDM;

////////////////////////////////////////////////////////////////////////////////

void LibRDM::initiate()
{
  Core::instance().root()
      ->get_child("Tools")
      ->create_component<RDM::ScalarAdvection>( "SetupScalarAdvection" )
      ->mark_basic();
}

void LibRDM::terminate()
{
  Core::instance().root()
      ->get_child("Tools")
      ->remove_component("SetupScalarAdvection");
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
