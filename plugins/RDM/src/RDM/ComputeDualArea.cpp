// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/CLink.hpp"
#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/Field.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/CellLoop.hpp"
#include "RDM/ComputeDualArea.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CellLoopT1< ComputeDualArea >, RDM::CellLoop, LibRDM > ComputeDualArea_CellLoop_Builder;

Common::ComponentBuilder < ComputeDualArea, Solver::Action, LibRDM > ComputeDualArea_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ComputeDualArea::ComputeDualArea ( const std::string& name ) : RDM::CellTerm(name)
{
  regist_typeinfo(this);
}

ComputeDualArea::~ComputeDualArea() {}

void ComputeDualArea::create_dual_area_field()
{
  CMesh& mymesh = mesh();
  Geometry& geometry = mymesh.geometry();

  // create if does not exist

  Field::Ptr comp = find_component_ptr_with_tag<Mesh::Field>( mymesh, Tags::dual_area());
  if( is_null( comp ) )
  {
    comp = geometry.create_field(Tags::dual_area(), "dual_area" ).as_ptr<Mesh::Field>();
    comp->add_tag(Tags::dual_area());
  }

  cdual_area = comp;

  RDM::RDSolver& mysolver = solver().as_type< RDM::RDSolver >();
  CGroup& fields = mysolver.fields();

  if( ! fields.get_child_ptr( Tags::dual_area() ) )
    fields.create_component<CLink>( Tags::dual_area() ).link_to(comp).add_tag( Tags::dual_area() );
}

void ComputeDualArea::execute()
{

  std::cout << "+++ Compute dual area +++" << std::endl;

  // ensure that the fields are present

  link_fields();

  create_dual_area_field();

  if( m_loop_regions.empty() )
    m_loop_regions.push_back( mesh().topology().as_ptr<CRegion>() );

  // get the element loop or create it if does not exist

  ElementLoop::Ptr loop;
  Common::Component::Ptr cloop = get_child_ptr( "LOOP" );
  if( is_null( cloop ) )
  {
    loop = build_component_abstract_type_reduced< CellLoop >( "CellLoopT1<" + type_name() + ">" , "LOOP");
    add_component(loop);
  }
  else
    loop = cloop->as_ptr_checked<ElementLoop>();

  // loop on all regions configured by the user

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {
    std::cout << "       -> Compute dual area in region [" << region->uri().string() << "]" << std::endl;

    loop->select_region( region );

    // loop all elements of this region

    loop->execute();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
