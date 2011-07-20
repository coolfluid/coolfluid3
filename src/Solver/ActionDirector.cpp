// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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


using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////////////////

ActionDirector::ActionDirector ( const std::string& name ) :
  Common::CActionDirector(name)
{
  mark_basic();

  // options

  m_options.add_option( OptionComponent<CSolver>::create("solver", &m_solver))
      ->set_description("Link to the solver discretizing the problem")
      ->set_pretty_name("Solver")
      ->mark_basic();

  m_options.add_option( OptionComponent<CMesh>::create("mesh", &m_mesh))
      ->set_description("Mesh the Discretization Method will be applied to")
      ->set_pretty_name("Mesh")
      ->mark_basic();

  m_options.add_option( OptionComponent<Physics::PhysModel>::create("physical_model", &m_physical_model))
      ->set_description("Physical model")
      ->set_pretty_name("Physical Model")
      ->mark_basic();

  m_options.add_option( OptionComponent<CTime>::create("time", &m_time))
      ->set_description("Time tracking component")
      ->set_pretty_name("Time")
      ->mark_basic();
}

ActionDirector::~ActionDirector() {}


Physics::PhysModel& ActionDirector::physical_model()
{
  Physics::PhysModel::Ptr model = m_physical_model.lock();
  if( is_null(model) )
    throw Common::SetupError( FromHere(),
                             "Physical Model not yet set for component " + uri().string() );
  return *model;
}


CTime& ActionDirector::time()
{
  CTime::Ptr t = m_time.lock();
  if( is_null(t) )
    throw Common::SetupError( FromHere(),
                             "Time not yet set for component " + uri().string() );
  return *t;
}


CMesh& ActionDirector::mesh()
{
  CMesh::Ptr m = m_mesh.lock();
  if( is_null(m) )
    throw Common::SetupError( FromHere(),
                             "Mesh not yet set for component " + uri().string() );
  return *m;
}


Solver::CSolver& ActionDirector::solver()
{
  Solver::CSolver::Ptr s = m_solver.lock();
  if( is_null(s) )
    throw Common::SetupError( FromHere(),
                             "Solver not yet set for component " + uri().string() );
  return *s;
}


////////////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
