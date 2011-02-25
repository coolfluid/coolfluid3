// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"

#include "FVM/LibFVM.hpp"

#include "FVM/ShockTube.hpp"

namespace CF {
namespace FVM {

  using namespace Common;
  
CF::Common::RegistLibrary<LibFVM> LibFVM;

////////////////////////////////////////////////////////////////////////////////

void LibFVM::initiate()
{
  Core::instance().root()
      ->get_child_ptr("Tools")
      ->create_component<FVM::ShockTube>( "wizard_shocktube" );
}

void LibFVM::terminate()
{
  Core::instance().root()
      ->get_child_ptr("Tools")
      ->remove_component("wizard_shocktube");
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
