// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"
#include "common/Root.hpp"
#include "common/Group.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/SteadyExplicit.hpp"
#include "RDM/UnsteadyExplicit.hpp"
#include "RDM/MySim.hpp"

namespace cf3 {
namespace RDM {


using namespace cf3::common;

cf3::common::RegistLibrary<LibRDM> LibRDM;

////////////////////////////////////////////////////////////////////////////////

void LibRDM::initiate_impl()
{
  Group& rdm_group =
      common::Core::instance().tools().create_component<Group>( "RDM" );
  rdm_group.mark_basic();

  rdm_group.create_component<RDM::SteadyExplicit>("Steady_Explicit_Wizard").mark_basic();
  rdm_group.create_component<RDM::UnsteadyExplicit>("Unsteady_Explicit_Wizard").mark_basic();
  rdm_group.create_component<RDM::MySim>( "MySim_Wizard" ).mark_basic();
}

void LibRDM::terminate_impl()
{
  Group& rdm_group =
      common::Core::instance().tools().get_child("RDM").as_type<Group>();

  rdm_group.remove_component( "Steady_Explicit_Wizard" );
  rdm_group.remove_component( "Unsteady_Explicit_Wizard" );
  rdm_group.remove_component( "MySim_Wizard" );

  common::Core::instance().tools().remove_component("RDM");
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
