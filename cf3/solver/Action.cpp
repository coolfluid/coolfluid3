// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/URI.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"

#include "physics/PhysModel.hpp"

#include "solver/CTime.hpp"
#include "solver/Action.hpp"
#include "solver/CSolver.hpp"
#include "solver/Tags.hpp"

using namespace cf3::mesh;

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////////////////

Action::Action ( const std::string& name ) :
  common::Action(name)
{
  mark_basic();

  // options

  options().add_option(Tags::solver(), m_solver)
      .description("Link to the solver discretizing the problem")
      .pretty_name("Solver")
      .mark_basic()
      .link_to(&m_solver);

  options().add_option(Tags::mesh(), m_mesh)
      .description("Mesh the Discretization Method will be applied to")
      .pretty_name("Mesh")
      .mark_basic()
      .link_to(&m_mesh);

  options().add_option(Tags::physical_model(), m_physical_model)
      .description("Physical model")
      .pretty_name("Physical Model")
      .mark_basic()
      .link_to(&m_physical_model);

  std::vector< common::URI > dummy;
  options().add_option(Tags::regions(), dummy)
      .description("Regions this action is applied to")
      .pretty_name("Regions")
      .attach_trigger ( boost::bind ( &Action::config_regions,   this ) );

}


Action::~Action() {}


physics::PhysModel& Action::physical_model()
{
  Handle< physics::PhysModel > model = m_physical_model;
  if( is_null(model) )
    throw common::SetupError( FromHere(),
                             "Physical Model not yet set for component " + uri().string() );
  return *model;
}



Mesh& Action::mesh()
{
  Handle< Mesh > m = m_mesh;
  if( is_null(m) )
    throw common::SetupError( FromHere(),
                             "Mesh not yet set for component " + uri().string() );
  return *m;
}


solver::CSolver& Action::solver()
{
  Handle< solver::CSolver > s = m_solver;
  if( is_null(s) )
    throw common::SetupError( FromHere(),
                             "Solver not yet set for component " + uri().string() );
  return *s;
}


const std::vector< Handle<Region> >& Action::regions() const
{
  return m_loop_regions;
}


void Action::config_regions()
{
  std::vector<common::URI> vec = options().option(Tags::regions()).value< std::vector<common::URI> >();

  m_loop_regions.clear();

  boost_foreach(const common::URI region_path, vec)
  {
    Handle<Component> comp = access_component(region_path);

    if ( Handle< Region > region = comp->handle<Region>() )
      m_loop_regions.push_back( region );
    else
      throw common::ValueNotFound ( FromHere(),
                           "Could not find region with path [" + region_path.path() +"]" );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
