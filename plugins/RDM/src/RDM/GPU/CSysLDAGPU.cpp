// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "Common/CBuilder.hpp"

#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"
//#include "Common/CreateComponent.hpp"

#include "Mesh/CRegion.hpp"

#include "Physics/PhysModel.hpp"

#include "RDM/CellLoop.hpp"
#include "RDM/GPU/CSysLDAGPU.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CSysLDAGPU, RDM::DomainTerm, LibSchemes > CSysLDAGPU_Builder;

////////////////////////////////////////////////////////////////////////////////

CSysLDAGPU::CSysLDAGPU ( const std::string& name ) : RDM::DomainTerm(name)
{
  regist_typeinfo(this);
}

CSysLDAGPU::~CSysLDAGPU() {}

void CSysLDAGPU::execute()
{
  // get the element loop or create it if does not exist
  ElementLoop::Ptr loop;
  Common::Component::Ptr cloop = get_child_ptr( "LOOP" );
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

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
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
