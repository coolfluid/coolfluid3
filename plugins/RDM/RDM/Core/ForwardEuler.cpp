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

#include "Math/MathChecks.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "Solver/Actions/CCriterionMaxIterations.hpp"

#include "RDM/Core/ForwardEuler.hpp"
#include "RDM/Core/UpdateSolution.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Math::MathChecks;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ForwardEuler, CAction, LibRDM > ForwardEuler_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
ForwardEuler::ForwardEuler ( const std::string& name ) :
  RDM::Action(name),
  m_cfl(1.0),
  m_max_iter(0)
{
  mark_basic();

  // property

  m_properties.add_property( "iteration", Uint(0) );

  // options

  m_properties.add_option< OptionT<Real> > ("CFL", "Courant Number", m_cfl)
      ->mark_basic()
      ->link_to(&m_cfl)
      ->add_tag("cfl");

  m_properties.add_option<OptionT <Uint> >("MaxIter",
                                           "Maximum number of iterations",
                                            m_max_iter)
      ->mark_basic()
      ->link_to( &m_max_iter );

  m_properties.add_option(OptionComponent<CField>::create("Solution","Solution field", &m_solution))
    ->add_tag("solution");

  m_properties.add_option(OptionComponent<CField>::create("WaveSpeed","Wave speed field", &m_wave_speed))
    ->add_tag("wave_speed");

  m_properties.add_option(OptionComponent<CField>::create("Residual","Residual field", &m_residual))
    ->add_tag("residual");

  create_static_component<CCriterionMaxIterations>("max_iterations");

  create_static_component<UpdateSolution>("update_solution");

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

  get_child("max_iterations").configure_property("iterative_step", full_path());

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

  // iteration loop

  Uint iteration = 1; // iterations start from 1 ( max iter zero will do nothing )
  property("iteration").change_value( iteration );

  while( !stop_condition() )
  {
    cleanup.execute(); // cleanup fields (typically residual and wave_speed)

    compute_boundary_terms.execute();

    compute_domain_terms.execute();

    /// @note consider if update solution should not make part of this action

    update_solution.execute();

    compute_norm.execute();

    /// @todo move this into an action
    //    m_output_convergence->execute();

    /// @todo move current rhs as a property of the iterate or solver components
    // output convergence info
    Real rhs_norm = compute_norm.property("Norm").value<Real>();
    CFinfo << "Iter [" << std::setw(4) << iteration << "] L2(rhs) [" << std::setw(12) << rhs_norm << "]" << CFendl;
   if ( is_nan(rhs_norm) || is_inf(rhs_norm) )
      throw FailedToConverge(FromHere(),"Solution diverged after "+to_str(iteration)+" iterations");

   property("iteration").change_value( ++iteration ); // update the iteration number

  }
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

