// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/URI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CTime.hpp"
#include "Solver/Action.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/Tags.hpp"

using namespace cf3::common;
using namespace cf3::Mesh;

namespace cf3 {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////////////////

Action::Action ( const std::string& name ) :
  common::CAction(name)
{
  mark_basic();

  // options

  m_options.add_option( OptionComponent<CSolver>::create(Tags::solver(), &m_solver))
      ->description("Link to the solver discretizing the problem")
      ->pretty_name("Solver")
      ->mark_basic();

  m_options.add_option( OptionComponent<CMesh>::create(Tags::mesh(), &m_mesh))
      ->description("Mesh the Discretization Method will be applied to")
      ->pretty_name("Mesh")
      ->mark_basic();

  m_options.add_option( OptionComponent<Physics::PhysModel>::create(Tags::physical_model(), &m_physical_model))
      ->description("Physical model")
      ->pretty_name("Physical Model")
      ->mark_basic();

  std::vector< URI > dummy;
  m_options.add_option< OptionArrayT<URI> > (Tags::regions(), dummy)
      ->description("Regions this action is applied to")
      ->pretty_name("Regions")
      ->attach_trigger ( boost::bind ( &Action::config_regions,   this ) );

}


Action::~Action() {}


Physics::PhysModel& Action::physical_model()
{
  Physics::PhysModel::Ptr model = m_physical_model.lock();
  if( is_null(model) )
    throw common::SetupError( FromHere(),
                             "Physical Model not yet set for component " + uri().string() );
  return *model;
}



CMesh& Action::mesh()
{
  CMesh::Ptr m = m_mesh.lock();
  if( is_null(m) )
    throw common::SetupError( FromHere(),
                             "Mesh not yet set for component " + uri().string() );
  return *m;
}


Solver::CSolver& Action::solver()
{
  Solver::CSolver::Ptr s = m_solver.lock();
  if( is_null(s) )
    throw common::SetupError( FromHere(),
                             "Solver not yet set for component " + uri().string() );
  return *s;
}


ComponentIteratorRange<CRegion> Action::regions()
{
  return ComponentIteratorRange<CRegion>(m_loop_regions);
}


void Action::config_regions()
{
  std::vector<URI> vec; option(Tags::regions()).put_value(vec);
  
  m_loop_regions.clear();

  boost_foreach(const URI region_path, vec)
  {
    Component& comp = access_component(region_path);

    if ( CRegion::Ptr region = comp.as_ptr<CRegion>() )
      m_loop_regions.push_back( region );
    else
      throw ValueNotFound ( FromHere(),
                           "Could not find region with path [" + region_path.path() +"]" );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // cf3
