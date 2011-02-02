// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"

#include "LibUFEM.hpp"
#include "SetupHeatConductionLinearSteady.hpp"

namespace CF {
namespace UFEM {

using namespace CF::Common;
  
CF::Common::RegistLibrary<LibUFEM> libUFEM;

////////////////////////////////////////////////////////////////////////////////

void LibUFEM::initiate()
{
  Core::instance().root()
    ->get_child("Tools")
    ->create_component<SetupHeatConductionLinearSteady>( "SetupHeatConduction" )
    ->mark_basic();
}

void LibUFEM::terminate()
{
  Core::instance().root()
      ->get_child("Tools")
      ->remove_component("SetupHeatConduction");
}

////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // CF
