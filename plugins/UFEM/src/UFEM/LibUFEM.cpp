// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"

#include "LibUFEM.hpp"

namespace CF {
namespace UFEM {

using namespace CF::Common;

CF::Common::RegistLibrary<LibUFEM> libUFEM;

////////////////////////////////////////////////////////////////////////////////

void LibUFEM::initiate_impl()
{
//   Core::instance().root()
//     .get_child_ptr("Tools")
//     ->create_component_ptr<SetupLinearSystem>( "SetupHeatConduction" )
//     ->mark_basic();


}

void LibUFEM::terminate_impl()
{
//   Core::instance().root()
//       .get_child_ptr("Tools")
//       ->remove_component("SetupHeatConduction");
}

////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // CF
