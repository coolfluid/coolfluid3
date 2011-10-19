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

using namespace cf3::common;
using namespace cf3::Mesh;
using namespace cf3::Solver;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CellLoopT1< ComputeDualArea >, RDM::CellLoop, LibRDM > ComputeDualArea_CellLoop_Builder;

common::ComponentBuilder < ComputeDualArea, Solver::Action, LibRDM > ComputeDualArea_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ComputeDualArea::ComputeDualArea ( const std::string& name ) : RDM::CellTerm(name)
{
  regist_typeinfo(this);
}

ComputeDualArea::~ComputeDualArea() {}

void ComputeDualArea::create_dual_area_field()
{
  RDM::RDSolver& rdsolver = solver().as_type< RDM::RDSolver >();
  CMesh& mymesh = mesh();

  const std::string solution_space = rdsolver.option("solution_space").value<std::string>();

  FieldGroup& solution_grp = find_component_with_tag<FieldGroup>( mymesh, solution_space );

  // create if does not exist

  Field::Ptr field;

  Component::Ptr comp = solution_grp.get_child_ptr( Tags::dual_area() );
  if( is_not_null( comp ) )
    field = comp->as_ptr_checked<Field>();
  else
  {
    field = solution_grp.create_field( Tags::dual_area(), "dual_area" ).as_ptr<Mesh::Field>();
    field->add_tag(Tags::dual_area());
  }

  cdual_area = field;

  RDM::RDSolver& mysolver = solver().as_type< RDM::RDSolver >();
  CGroup& fields = mysolver.fields();

  if( ! fields.get_child_ptr( Tags::dual_area() ) )
    fields.create_component<CLink>( Tags::dual_area() ).link_to(field).add_tag( Tags::dual_area() );
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
  common::Component::Ptr cloop = get_child_ptr( "LOOP" );
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
} // cf3
