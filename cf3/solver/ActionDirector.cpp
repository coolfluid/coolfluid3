// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/URI.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"

#include "physics/PhysModel.hpp"

#include "solver/LibSolver.hpp"
#include "solver/Time.hpp"
#include "solver/ActionDirector.hpp"
#include "solver/Solver.hpp"
#include "solver/Tags.hpp"

namespace cf3 {
  common::ComponentBuilder < solver::ActionDirector, common::ActionDirector, solver::LibSolver > SolverActionDirector_Builder;
namespace solver {


using namespace cf3::common;
using namespace cf3::mesh;
  
////////////////////////////////////////////////////////////////////////////////////////////

ActionDirector::ActionDirector ( const std::string& name ) :
  common::ActionDirector(name)
{
  mark_basic();

  // options

  options().add(Tags::solver(), m_solver)
      .description("Link to the solver discretizing the problem")
      .pretty_name("Solver")
      .mark_basic()
      .link_to(&m_solver);

  options().add("mesh", m_mesh)
      .description("Mesh the Discretization Method will be applied to")
      .pretty_name("Mesh")
      .mark_basic()
      .link_to(&m_mesh);

  options().add(Tags::physical_model(), m_physical_model)
      .description("Physical model")
      .pretty_name("Physical Model")
      .mark_basic()
      .link_to(&m_physical_model);
      
  std::vector< common::URI > dummy;
  options().add(Tags::regions(), dummy)
      .description("Regions this action is applied to")
      .pretty_name("Regions")
      .attach_trigger ( boost::bind ( &ActionDirector::config_regions,   this ) )
      .mark_basic();
}

ActionDirector::~ActionDirector() {}


physics::PhysModel& ActionDirector::physical_model()
{
  if( is_null(m_physical_model) )
    throw common::SetupError( FromHere(),
                             "Physical Model not yet set for component " + uri().string() );
  return *m_physical_model;
}


Mesh& ActionDirector::mesh()
{
  if( is_null(m_mesh) )
    throw common::SetupError( FromHere(),
                             "Mesh not yet set for component " + uri().string() );
  return *m_mesh;
}


solver::Solver& ActionDirector::solver()
{
  if( is_null(m_solver) )
    throw common::SetupError( FromHere(),
                             "Solver not yet set for component " + uri().string() );
  return *m_solver;
}

void ActionDirector::config_regions()
{
  m_loop_regions.clear();

  const std::string regions_option_name("regions"); // for Intel Composer 2011...
  boost_foreach(const common::URI region_uri, options().option(regions_option_name).value< std::vector<common::URI> >())
  {
    Handle<Component> comp;
    if (region_uri.is_relative())
    {
      if ( is_null(m_mesh) )
        throw common::SetupError(FromHere(), "First configure the mesh");
      comp = m_mesh->access_component(region_uri);
    }
    else
    {
      comp = access_component(region_uri);
    }

    if ( is_null(comp) )
    {
      throw common::ValueNotFound ( FromHere(),
                           "Could not find region with path [" + region_uri.path() +"]" );
    }
    Handle< Region > region = comp->handle<Region>();
    if ( is_not_null(region) )
      m_loop_regions.push_back( region );
    else
      throw common::ValueNotFound ( FromHere(),
                           "Component [" + region_uri.path() +"] is not of type Region" );
  }
  
  on_regions_set();
}

void ActionDirector::on_regions_set()
{
}


////////////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
