// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/URI.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/Mesh.hpp"

#include "physics/PhysModel.hpp"

#include "solver/CTime.hpp"
#include "solver/ActionDirector.hpp"
#include "solver/CSolver.hpp"
#include "solver/Tags.hpp"


using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////////////////

ActionDirector::ActionDirector ( const std::string& name ) :
  common::ActionDirector(name)
{
  mark_basic();

  // options

  m_options.add_option( OptionComponent<CSolver>::create(Tags::solver(), &m_solver))
      ->description("Link to the solver discretizing the problem")
      ->pretty_name("Solver")
      ->mark_basic();

  m_options.add_option( OptionComponent<Mesh>::create("mesh", &m_mesh))
      ->description("Mesh the Discretization Method will be applied to")
      ->pretty_name("Mesh")
      ->mark_basic();

  m_options.add_option( OptionComponent<physics::PhysModel>::create(Tags::physical_model(), &m_physical_model))
      ->description("Physical model")
      ->pretty_name("Physical Model")
      ->mark_basic();

  m_options.add_option( OptionComponent<CTime>::create(Tags::time(), &m_time))
      ->description("Time tracking component")
      ->pretty_name("Time")
      ->mark_basic();
}

ActionDirector::~ActionDirector() {}


physics::PhysModel& ActionDirector::physical_model()
{
  physics::PhysModel::Ptr model = m_physical_model.lock();
  if( is_null(model) )
    throw common::SetupError( FromHere(),
                             "Physical Model not yet set for component " + uri().string() );
  return *model;
}


CTime& ActionDirector::time()
{
  CTime::Ptr t = m_time.lock();
  if( is_null(t) )
    throw common::SetupError( FromHere(),
                             "Time not yet set for component " + uri().string() );
  return *t;
}


Mesh& ActionDirector::mesh()
{
  Mesh::Ptr m = m_mesh.lock();
  if( is_null(m) )
    throw common::SetupError( FromHere(),
                             "Mesh not yet set for component " + uri().string() );
  return *m;
}


solver::CSolver& ActionDirector::solver()
{
  solver::CSolver::Ptr s = m_solver.lock();
  if( is_null(s) )
    throw common::SetupError( FromHere(),
                             "Solver not yet set for component " + uri().string() );
  return *s;
}


////////////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
