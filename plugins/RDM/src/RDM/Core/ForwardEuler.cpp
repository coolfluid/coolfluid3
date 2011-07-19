// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/EventHandler.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Math/Checks.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "Solver/Actions/CCriterionMaxIterations.hpp"

#include "RDM/Core/ForwardEuler.hpp"
#include "RDM/Core/UpdateSolution.hpp"

// #include "Common/MPI/debug.hpp" // temporary

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Math::Checks;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ForwardEuler, CAction, LibCore > ForwardEuler_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ForwardEuler::ForwardEuler ( const std::string& name ) :
  Solver::Action(name),
  m_cfl(1.0),
  m_max_iter(0)
{
  mark_basic();

  // property

  m_properties.add_property( "iteration", Uint(0) );

  // options

  m_options.add_option< OptionT<Real> > ("cfl", m_cfl)
      ->set_description("Courant Number")
      ->set_pretty_name("CFL")
      ->mark_basic()
      ->link_to(&m_cfl);

  m_options.add_option<OptionT <Uint> >("MaxIter", m_max_iter)
      ->set_description("Maximum number of iterations")
      ->mark_basic()
      ->link_to( &m_max_iter );

  m_options.add_option(OptionComponent<CField>::create("solution", &m_solution))
      ->set_description("Solution field")
      ->set_pretty_name("Solution");

  m_options.add_option(OptionComponent<CField>::create("wave_speed", &m_wave_speed))
      ->set_description("Wave speed field")
      ->set_pretty_name("WaveSpeed");

  m_options.add_option(OptionComponent<CField>::create("residual", &m_residual))
      ->set_description("Residual field");

  create_static_component_ptr<CCriterionMaxIterations>("max_iterations");

  create_static_component_ptr<UpdateSolution>("update_solution");

}

////////////////////////////////////////////////////////////////////////////////

bool ForwardEuler::stop_condition()
{
  bool finish = false;
  boost_foreach(CCriterion& stop_criterion, find_components<CCriterion>(*this))
      finish |= stop_criterion();
  return finish;
}

void ForwardEuler::execute()
{
  if (m_solution.expired())   throw SetupError(FromHere(), "Solution field was not set");
  if (m_wave_speed.expired()) throw SetupError(FromHere(), "WaveSpeed Field was not set");
  if (m_residual.expired())   throw SetupError(FromHere(), "Residual field was not set");

  get_child("max_iterations").configure_option("iterator", uri());

//  CTable<Real>& solution     = m_solution.lock()->data();
//  CTable<Real>& wave_speed   = m_wave_speed.lock()->data();
//  CTable<Real>& residual     = m_residual.lock()->data();

  Common::CAction& cleanup =
      access_component( "cpath:../cleanup" ).as_type<CAction>();
  Common::CAction& compute_boundary_terms =
      access_component( "cpath:../compute_boundary_terms" ).as_type<CAction>();
  Common::CAction& compute_domain_terms =
      access_component( "cpath:../compute_domain_terms" ).as_type<CAction>();
  Common::CAction& compute_norm =
      access_component( "cpath:../compute_norm" ).as_type<CAction>();

  Common::CAction& update_solution =
      get_child("update_solution").as_type<CAction>();


//  std::cout << PERank << " synchronizing initial solution ... " << std::endl;
  m_solution.lock()->synchronize(); // parallel synchronization


  // iteration loop

  Uint iteration = 1; // iterations start from 1 ( max iter zero will do nothing )
  property("iteration") = iteration;

  while( !stop_condition() )
  {
//    std::cout << PERank << " cleanup solution ... " << std::endl;

    cleanup.execute(); // cleanup fields (typically residual and wave_speed)

//    std::cout << PERank << " apply bcs ... " << std::endl;

    compute_boundary_terms.execute();

//    std::cout << PERank << " computing domain terms ... " << std::endl;

    compute_domain_terms.execute();

    /// @note consider if update solution should not make part of this action

//    std::cout << PERank << " updating solution ... " << std::endl;

    update_solution.execute();

//    std::cout << PERank << " synchronizing solution ... " << std::endl;

    m_solution.lock()->synchronize(); // parallel synchronization

//    std::cout << PERank << " synchronizing solution ...  done " << std::endl;

    compute_norm.execute();

    /// @todo move this into an action
    //    m_output_convergence->execute();

    /// @todo move current rhs as a property of the iterate or solver components
    // output convergence info
    Real rhs_norm = compute_norm.properties().value<Real>("Norm");
    std::cout << " Iter [" << std::setw(4) << iteration << "]"
              << " L2(rhs) [" << std::setw(12) << rhs_norm << "]" << std::endl;

    if ( is_nan(rhs_norm) || is_inf(rhs_norm) )
      throw FailedToConverge(FromHere(),
                             "Solution diverged after "+to_str(iteration)+" iterations");

   // raise signal that iteration is done
   /// @todo move this to an Action and/or separate function in base class

   SignalOptions opts;
   SignalFrame frame;

   opts.add_option< OptionT<Uint> >( "iteration", iteration );

   frame = opts.create_frame("iteration_done", URI(), URI());

   Core::instance().event_handler().raise_event( "iteration_done", frame);

   // increment iteration

   property("iteration") = ++iteration; // update the iteration number

  }
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

