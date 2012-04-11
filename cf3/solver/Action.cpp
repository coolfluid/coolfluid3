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

#include "solver/Time.hpp"
#include "solver/Action.hpp"
#include "solver/Solver.hpp"
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


solver::Solver& Action::solver()
{
  Handle< solver::Solver > s = m_solver;
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
}

////////////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
