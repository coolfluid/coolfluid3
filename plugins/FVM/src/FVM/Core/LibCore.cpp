// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"

#include "FVM/Core/LibCore.hpp"

#include "FVM/Core/ShockTube.hpp"

namespace CF {
namespace FVM {
namespace Core {

  using namespace Common;

CF::Common::RegistLibrary<LibCore> LibCore;

////////////////////////////////////////////////////////////////////////////////

void LibCore::initiate_impl()
{
  Common::Core::instance().root()
      .get_child_ptr("Tools")
      ->create_component<FVM::Core::ShockTube>( "wizard_shocktube" );
}

void LibCore::terminate_impl()
{
  Common::Core::instance().root()
      .get_child_ptr("Tools")
      ->remove_component("wizard_shocktube");
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF
