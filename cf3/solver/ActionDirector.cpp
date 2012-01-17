// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/URI.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"

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

  options().add_option(Tags::solver(), m_solver)
      .description("Link to the solver discretizing the problem")
      .pretty_name("Solver")
      .mark_basic()
      .link_to(&m_solver);

  options().add_option("mesh", m_mesh)
      .description("Mesh the Discretization Method will be applied to")
      .pretty_name("Mesh")
      .mark_basic()
      .link_to(&m_mesh);

  options().add_option(Tags::physical_model(), m_physical_model)
      .description("Physical model")
      .pretty_name("Physical Model")
      .mark_basic()
      .link_to(&m_physical_model);

  options().add_option(Tags::time(), m_time)
      .description("Time tracking component")
      .pretty_name("Time")
      .mark_basic()
      .link_to(&m_time);
}

ActionDirector::~ActionDirector() {}


physics::PhysModel& ActionDirector::physical_model()
{
  if( is_null(m_physical_model) )
    throw common::SetupError( FromHere(),
                             "Physical Model not yet set for component " + uri().string() );
  return *m_physical_model;
}


CTime& ActionDirector::time()
{
  if( is_null(m_time) )
    throw common::SetupError( FromHere(),
                             "Time not yet set for component " + uri().string() );
  return *m_time;
}


Mesh& ActionDirector::mesh()
{
  if( is_null(m_mesh) )
    throw common::SetupError( FromHere(),
                             "Mesh not yet set for component " + uri().string() );
  return *m_mesh;
}


solver::CSolver& ActionDirector::solver()
{
  if( is_null(m_solver) )
    throw common::SetupError( FromHere(),
                             "Solver not yet set for component " + uri().string() );
  return *m_solver;
}


////////////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
