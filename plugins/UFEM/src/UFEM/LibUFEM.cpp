// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"
#include "common/CRoot.hpp"

#include "LibUFEM.hpp"

namespace cf3 {
namespace UFEM {

using namespace cf3::common;

cf3::common::RegistLibrary<LibUFEM> libUFEM;

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
} // cf3
