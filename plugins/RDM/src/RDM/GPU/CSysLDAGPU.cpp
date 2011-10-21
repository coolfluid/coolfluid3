// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "common/Builder.hpp"

#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
//#include "common/CreateComponent.hpp"

#include "mesh/Region.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "RDM/Tags.hpp"
#include "RDM/GPU/CellLoopGPU.hpp"
#include "RDM/GPU/CSysLDAGPU.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::Solver;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CSysLDAGPU, RDM::CellTerm, LibGPU > CSysLDAGPU_Builder;

////////////////////////////////////////////////////////////////////////////////

CSysLDAGPU::CSysLDAGPU ( const std::string& name ) : RDM::CellTerm(name)
{
  regist_typeinfo(this);
}

CSysLDAGPU::~CSysLDAGPU() {}

void CSysLDAGPU::execute()
{
  // get the element loop or create it if does not exist
  ElementLoop::Ptr loop;
  common::Component::Ptr cloop = get_child_ptr( "LOOP" );
  if( is_null( cloop ) )
  {
    const std::string update_vars_type =
        physical_model().get_child( RDM::Tags::update_vars() )
                        .as_type<Physics::Variables>()
                        .type();

      loop = build_component_abstract_type_reduced< CellLoop >( "CellLoopGPU<" + type_name() + "," + update_vars_type + ">" , "LOOP");
      add_component(loop);
  }
  else
    loop = cloop->as_ptr_checked<ElementLoop>();

  // loop on all regions configured by the user

  boost_foreach(mesh::Region::Ptr& region, m_loop_regions)
  {
    std::cout << "looping on region " << region->name() << std::endl;

    loop->select_region( region );

    // loop all elements of this region

    loop->execute();
  }
}

//////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
