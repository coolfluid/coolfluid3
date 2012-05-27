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
#include "common/Signal.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/PropertyList.hpp"

#include "math/Consts.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"

#include "solver/ModelUnsteady.hpp"
#include "solver/Solver.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

namespace cf3 {
namespace solver {

using namespace common;
using namespace mesh;

common::ComponentBuilder < ModelUnsteady, Component, LibSolver > ModelUnsteady_Builder;

////////////////////////////////////////////////////////////////////////////////

struct ModelUnsteady::Implementation
{
  Implementation(Component& component) :
    m_component(component)
  {
  }

  Component& m_component;
  Handle<Time> m_time;
};

////////////////////////////////////////////////////////////////////////////////

ModelUnsteady::ModelUnsteady( const std::string& name  ) :
  Model ( name ),
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

  regist_signal( "create_time" )
    .connect( boost::bind( &ModelUnsteady::signal_create_time, this, _1 ) )
    .description("Create the time tracking component")
    .pretty_name("Create Time");

}

ModelUnsteady::~ModelUnsteady() {}

void ModelUnsteady::setup(const std::string& solver_builder_name, const std::string& physics_builder_name)
{
  cf3::solver::Model::setup(solver_builder_name, physics_builder_name);
  create_time("Time");
}


void ModelUnsteady::simulate ()
{
  Model::simulate();
//  time().options().set("time", time().current_time() );
}


Time& ModelUnsteady::create_time(const std::string& name)
{

  Handle<Time> time = create_component<Time>(name);
  m_implementation->m_time = time;

  configure_option_recursively(Tags::time(), time);

  return *time;
}

void ModelUnsteady::signal_create_time ( common::SignalArgs& node )
{
  Time& time = create_time("Time");

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", time.uri());
}

Time& ModelUnsteady::time()
{
  if(is_null(m_implementation->m_time))
    throw SetupError(FromHere(), "Time is not configured for model " + uri().string());

  return *m_implementation->m_time;
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
