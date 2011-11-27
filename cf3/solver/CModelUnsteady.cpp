// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/PropertyList.hpp"

#include "math/Consts.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"

#include "solver/CModelUnsteady.hpp"
#include "solver/CSolver.hpp"
#include "solver/CTime.hpp"
#include "solver/Tags.hpp"

namespace cf3 {
namespace solver {

using namespace common;
using namespace mesh;

common::ComponentBuilder < CModelUnsteady, Component, LibSolver > CModelUnsteady_Builder;

////////////////////////////////////////////////////////////////////////////////

struct CModelUnsteady::Implementation
{
  Implementation(Component& component) :
    m_component(component)
  {
  }

  Component& m_component;
  Handle<CTime> m_time;
};

////////////////////////////////////////////////////////////////////////////////

CModelUnsteady::CModelUnsteady( const std::string& name  ) :
  CModel ( name ),
  m_implementation(new Implementation(*this))
{
  properties()["steady"] = bool(false);

  properties()["brief"] = std::string("Unsteady simulator object");
  std::string description =
  "This object handles unsteady time accurate simulations.\n"
  "The simulator consists of some specific components:\n"
  " - \"domain\" which specifies 1 or more geometries used in the simulation.\n"
  " - \"time\" which holds track of time steps and simulation time.\n"
  " - \"physics\" which define the physics of the problem, equations, ...\n"
  " - \"iterative solver\" which will advance the solution in time\n"
  "   The iterative solver delegates space discretization to a \"discretization method\"";
  properties()["description"] = description;

}

CModelUnsteady::~CModelUnsteady() {}

void CModelUnsteady::setup(const std::string& solver_builder_name, const std::string& physics_builder_name)
{
  cf3::solver::CModel::setup(solver_builder_name, physics_builder_name);
  create_time("Time");
}


void CModelUnsteady::simulate ()
{
  CModel::simulate();
//  time().options().configure_option("time", time().current_time() );
}


CTime& CModelUnsteady::create_time(const std::string& name)
{
  Handle<CTime> time = create_component<CTime>(name);
  m_implementation->m_time = time;

  configure_option_recursively(Tags::time(), time->uri());

  return *time;
}


void CModelUnsteady::signal_create_time(SignalArgs node)
{
  create_time();
}


CTime& CModelUnsteady::time()
{
  if(is_null(m_implementation->m_time))
    throw SetupError(FromHere(), "Time is not configured for model " + uri().string());

  return *m_implementation->m_time;
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
