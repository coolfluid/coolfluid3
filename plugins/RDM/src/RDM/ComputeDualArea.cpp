// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Log.hpp"

#include "common/Builder.hpp"
#include "common/Link.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"

#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/CellLoop.hpp"
#include "RDM/ComputeDualArea.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CellLoopT1< ComputeDualArea >, RDM::CellLoop, LibRDM > ComputeDualArea_CellLoop_Builder;

common::ComponentBuilder < ComputeDualArea, common::Action, LibRDM > ComputeDualArea_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ComputeDualArea::ComputeDualArea ( const std::string& name ) : RDM::CellTerm(name)
{
  regist_typeinfo(this);
}

ComputeDualArea::~ComputeDualArea() {}

void ComputeDualArea::create_dual_area_field()
{
  RDM::RDSolver& rdsolver = *solver().handle< RDM::RDSolver >();
  Mesh& mymesh = mesh();

  const std::string solution_space = rdsolver.options().value<std::string>("solution_space");

  Dictionary& solution_grp = find_component_with_tag<Dictionary>( mymesh, solution_space );

  // create if does not exist
  Handle< Field > field( solution_grp.get_child( Tags::dual_area() ) );
  if( is_null( field ) )
  {
    field = solution_grp.create_field( Tags::dual_area(), "dual_area" ).handle<mesh::Field>();
    field->add_tag(Tags::dual_area());
  }

  cdual_area = field;

  RDM::RDSolver& mysolver = *solver().handle< RDM::RDSolver >();
  Group& fields = mysolver.fields();

  if( ! fields.get_child( Tags::dual_area() ) )
    fields.create_component<Link>( Tags::dual_area() )->link_to(*field).add_tag( Tags::dual_area() );
  else
    if (mysolver.switch_to_sol)
      fields.get_child( Tags::dual_area() )->handle<Link>()->link_to(*field).add_tag( Tags::dual_area() );

CFinfo << "DUAL AREA: " << cdual_area->uri().name() << "  " << cdual_area->size() << CFendl << CFflush;
}

void ComputeDualArea::execute()
{

  std::cout << "+++ Compute dual area +++" << std::endl;

  // ensure that the fields are present

  link_fields();

  create_dual_area_field();

  if( m_loop_regions.empty() )
    m_loop_regions.push_back( mesh().topology().handle<Region>() );

  // get the element loop or create it if does not exist

  Handle< ElementLoop > loop(get_child( "LOOP" ));

  if( is_null( loop ) )
    loop = create_component<CellLoop>("LOOP", "CellLoopT1<" + type_name() + ">");

  // loop on all regions configured by the user

  boost_foreach(Handle< mesh::Region >& region, m_loop_regions)
  {
    std::cout << "       -> Compute dual area in region [" << region->uri().string() << "]" << std::endl;
    loop->select_region( region );
    // loop all elements of this region
    loop->execute();
  }

//  Handle< mesh::Field > coord =  mesh().geometry_fields().coordinates().handle< mesh::Field >();
//  CFinfo << "COOR: " << coord->uri().path() << "  " << coord->size() << CFflush << CFendl;
//  mesh::Field& coord=mesh().access_component_checked(URI("//Model/Domain/mesh/solution/coordinates"))->handle();
//  CFinfo << "COOR: " << coord.uri().path() << "  " << coord.size() << CFflush << CFendl;
//  Field& daf=dual_area();
//  CFinfo << "NAME: " << daf.uri().path() << "  " << daf.size() << CFflush << CFendl;
//  for (Uint n=0; n<daf.size(); ++n)
//    for (Uint j=0; j<daf.row_size(); ++j)
//      CFinfo << daf[n][j] << CFendl << CFflush;
//  Handle<Component> ccoord=root()->access_component_checked(URI("//Model/Domain/mesh/solution/coordinates"));
//  Handle<mesh::Field> coord=ccoord->handle<mesh::Field>();
//  CFinfo << "COOR: " << coord->uri().path() << "  " << coord->size() << CFflush << CFendl;


}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
