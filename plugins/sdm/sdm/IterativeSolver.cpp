// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionComponent.hpp"
#include "common/ActionDirector.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"

#include "math/VariablesDescriptor.hpp"

#include "solver/Time.hpp"
#include "solver/Solver.hpp"

#include "solver/actions/Criterion.hpp"
#include "solver/actions/CriterionMaxIterations.hpp"
#include "solver/actions/ComputeLNorm.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "sdm/IterativeSolver.hpp"
#include "sdm/Tags.hpp"
#include "sdm/SDSolver.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::mesh;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

IterativeSolver::IterativeSolver ( const std::string& name ) :
  solver::Action(name)
{
  mark_basic();

  // properties

  properties().add( "iteration", Uint(0) );

  // static components

  m_pre_update = create_static_component<common::ActionDirector>("PreUpdate");

  m_post_update = create_static_component<common::ActionDirector>("PostUpdate");

  options().add(sdm::Tags::solution(), m_solution)
      .description("Solution to update")
      .pretty_name("Solution")
      .link_to(&m_solution);

      options().add(sdm::Tags::update_coeff(), m_update_coeff)
      .description("Update coefficient")
      .pretty_name("Update Coefficient")
      .link_to(&m_update_coeff);

      options().add(sdm::Tags::residual(), m_residual)
      .description("Residual")
      .pretty_name("Residual")
      .link_to(&m_residual);

  options().add(sdm::Tags::time(), m_time)
      .description("Time component")
      .pretty_name("Time")
      .link_to(&m_time);

  ComputeLNorm& cnorm = *create_static_component<ComputeLNorm>( "ComputeNorm" );
  cnorm.options().set("order",2u);
  cnorm.options().set("scale",true);
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::link_fields()
{
  if (is_null(m_solution))
    m_solution = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::solution() ) ) );
  if (is_null(m_residual))
    m_residual = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::residual() ) ) );
  if (is_null(m_update_coeff))
    m_update_coeff = Handle<Field>( follow_link( solver().field_manager().get_child( sdm::Tags::update_coeff() ) ) );
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());
  common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
