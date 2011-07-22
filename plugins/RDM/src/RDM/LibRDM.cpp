// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"
#include "Common/CGroup.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/SteadyExplicit.hpp"
#include "RDM/UnsteadyExplicit.hpp"
#include "RDM/MySim.hpp"

namespace CF {
namespace RDM {


using namespace CF::Common;

CF::Common::RegistLibrary<LibRDM> LibRDM;

////////////////////////////////////////////////////////////////////////////////

void LibRDM::initiate_impl()
{
  CGroup& rdm_group =
      Common::Core::instance().tools().create_component<CGroup>( "RDM" );
  rdm_group.mark_basic();

  rdm_group.create_component<RDM::SteadyExplicit>("Steady_Explicit_Wizard").mark_basic();
  rdm_group.create_component<RDM::UnsteadyExplicit>("Unsteady_Explicit_Wizard").mark_basic();
  rdm_group.create_component<RDM::MySim>( "MySim_Wizard" ).mark_basic();
}

void LibRDM::terminate_impl()
{
  CGroup& rdm_group =
      Common::Core::instance().tools().get_child("RDM").as_type<CGroup>();

  rdm_group.remove_component( "Steady_Explicit_Wizard" );
  rdm_group.remove_component( "Unsteady_Explicit_Wizard" );
  rdm_group.remove_component( "MySim_Wizard" );

  Common::Core::instance().tools().remove_component("RDM");
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
