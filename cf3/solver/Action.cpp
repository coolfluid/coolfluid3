// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/URI.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"

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

  m_options.add_option( common::OptionComponent<CSolver>::create(Tags::solver(), &m_solver))
      ->description("Link to the solver discretizing the problem")
      ->pretty_name("Solver")
      ->mark_basic();

  m_options.add_option( common::OptionComponent<Mesh>::create(Tags::mesh(), &m_mesh))
      ->description("Mesh the Discretization Method will be applied to")
      ->pretty_name("Mesh")
      ->mark_basic();

  m_options.add_option( common::OptionComponent<physics::PhysModel>::create(Tags::physical_model(), &m_physical_model))
      ->description("Physical model")
      ->pretty_name("Physical Model")
      ->mark_basic();

  std::vector< common::URI > dummy;
  m_options.add_option< common::OptionArrayT<common::URI> > (Tags::regions(), dummy)
      ->description("Regions this action is applied to")
      ->pretty_name("Regions")
      ->attach_trigger ( boost::bind ( &Action::config_regions,   this ) );

}


Action::~Action() {}


physics::PhysModel& Action::physical_model()
{
  physics::PhysModel::Ptr model = m_physical_model.lock();
  if( is_null(model) )
    throw common::SetupError( FromHere(),
                             "Physical Model not yet set for component " + uri().string() );
  return *model;
}



Mesh& Action::mesh()
{
  Mesh::Ptr m = m_mesh.lock();
  if( is_null(m) )
    throw common::SetupError( FromHere(),
                             "Mesh not yet set for component " + uri().string() );
  return *m;
}


solver::CSolver& Action::solver()
{
  solver::CSolver::Ptr s = m_solver.lock();
  if( is_null(s) )
    throw common::SetupError( FromHere(),
                             "Solver not yet set for component " + uri().string() );
  return *s;
}


common::ComponentIteratorRange<Region> Action::regions()
{
  return common::ComponentIteratorRange<Region>(m_loop_regions);
}


void Action::config_regions()
{
  std::vector<common::URI> vec; option(Tags::regions()).put_value(vec);

  m_loop_regions.clear();

  boost_foreach(const common::URI region_path, vec)
  {
    Component& comp = access_component(region_path);

    if ( Region::Ptr region = comp.as_ptr<Region>() )
      m_loop_regions.push_back( region );
    else
      throw common::ValueNotFound ( FromHere(),
                           "Could not find region with path [" + region_path.path() +"]" );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
