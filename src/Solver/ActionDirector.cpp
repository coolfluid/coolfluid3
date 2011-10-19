// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/URI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CTime.hpp"
#include "Solver/ActionDirector.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/Tags.hpp"


using namespace cf3::common;
using namespace cf3::Mesh;

namespace cf3 {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////////////////

ActionDirector::ActionDirector ( const std::string& name ) :
  common::CActionDirector(name)
{
  mark_basic();

  // options

  m_options.add_option( OptionComponent<CSolver>::create(Tags::solver(), &m_solver))
      ->description("Link to the solver discretizing the problem")
      ->pretty_name("Solver")
      ->mark_basic();

  m_options.add_option( OptionComponent<CMesh>::create("mesh", &m_mesh))
      ->description("Mesh the Discretization Method will be applied to")
      ->pretty_name("Mesh")
      ->mark_basic();

  m_options.add_option( OptionComponent<Physics::PhysModel>::create(Tags::physical_model(), &m_physical_model))
      ->description("Physical model")
      ->pretty_name("Physical Model")
      ->mark_basic();

  m_options.add_option( OptionComponent<CTime>::create(Tags::time(), &m_time))
      ->description("Time tracking component")
      ->pretty_name("Time")
      ->mark_basic();
}

ActionDirector::~ActionDirector() {}


Physics::PhysModel& ActionDirector::physical_model()
{
  Physics::PhysModel::Ptr model = m_physical_model.lock();
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


CMesh& ActionDirector::mesh()
{
  CMesh::Ptr m = m_mesh.lock();
  if( is_null(m) )
    throw common::SetupError( FromHere(),
                             "Mesh not yet set for component " + uri().string() );
  return *m;
}


Solver::CSolver& ActionDirector::solver()
{
  Solver::CSolver::Ptr s = m_solver.lock();
  if( is_null(s) )
    throw common::SetupError( FromHere(),
                             "Solver not yet set for component " + uri().string() );
  return *s;
}


////////////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // cf3
