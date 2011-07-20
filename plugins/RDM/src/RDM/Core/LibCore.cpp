// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"
#include "Common/CGroup.hpp"

#include "RDM/Core/LibRDM.hpp"
#include "RDM/Core/SteadyExplicit.hpp"
#include "RDM/Core/MySim.hpp"

namespace CF {
namespace RDM {


using namespace CF::Common;

CF::Common::RegistLibrary<LibRDM> LibRDM;

////////////////////////////////////////////////////////////////////////////////

void LibRDM::initiate_impl()
{
  CGroup::Ptr rdm_group =
    Common::Core::instance().root()
      .get_child_ptr("Tools")
      ->create_component_ptr<CGroup>( "RDM" );
  rdm_group->mark_basic();

  rdm_group->create_component_ptr<RDM::SteadyExplicit>( "Setup_RD_Steady_Explicit" )
      ->mark_basic();
  rdm_group->create_component_ptr<RDM::MySim>( "Setup_RD_My_Sim" )
      ->mark_basic();
}

void LibRDM::terminate_impl()
{
  Common::Core::instance().root()
      .get_child_ptr("Tools")
      ->get_child_ptr("RDM")
      ->remove_component( "Setup_RD_Steady_Explicit" );
  Common::Core::instance().root()
      .get_child_ptr("Tools")
      ->get_child_ptr("RDM")
      ->remove_component( "Setup_RD_My_Sim" );
  Common::Core::instance().root()
      .get_child_ptr("Tools")
      ->remove_component("RDM");
}

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF
