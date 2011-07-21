// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/EventHandler.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Math/Checks.hpp"

#include "Solver/Actions/CSynchronizeFields.hpp"
#include "Solver/Actions/CCriterionMaxIterations.hpp"
#include "Solver/Actions/CComputeLNorm.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/Cleanup.hpp"

#include "IterativeSolver.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Math::Checks;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace RDM {


///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < IterativeSolver, CAction, LibRDM > IterativeSolver_Builder;

///////////////////////////////////////////////////////////////////////////////////////

IterativeSolver::IterativeSolver ( const std::string& name ) :
  CF::Solver::ActionDirector(name)
{
  mark_basic();

  // static components

  m_pre_actions  = create_static_component_ptr<CActionDirector>("PreActions");

  m_update = create_static_component_ptr<CActionDirector>("Update");

  m_post_actions = create_static_component_ptr<CActionDirector>("PostActions");

  // dynamic components

  Cleanup::Ptr cleanup  = m_pre_actions->create_component_ptr<Cleanup>("Zero");

  m_pre_actions->append( cleanup );

  CCriterionMaxIterations& maxiter =
      create_component<CCriterionMaxIterations>( "MaxIterations" );

  CComputeLNorm& cnorm =
      m_post_actions->create_component<CComputeLNorm>( "ComputeNorm" );

  m_post_actions->append( cnorm );

  cnorm.configure_option("Scale", true);
  cnorm.configure_option("Order", 2u);

  // properties

  m_properties.add_property( "iteration", Uint(0) );

}

bool IterativeSolver::stop_condition()
{
  bool finish = false;
  boost_foreach(CCriterion& stop_criterion, find_components<CCriterion>(*this))
      finish |= stop_criterion();
  return finish;
}


void IterativeSolver::execute()
{
  RDM::RDSolver& mysolver = solver().as_type< RDM::RDSolver >();

  //----------------------------------------------------------------------------------------

  /// @todo this configuration sould be in constructor but does not work there

  get_child("MaxIterations").configure_option( "iterator", this->uri() );

  std::vector<URI> cleanup_fields;
  cleanup_fields.push_back( mysolver.fields().get_child( RDM::Tags::residual()).follow()->uri() );
  cleanup_fields.push_back( mysolver.fields().get_child( RDM::Tags::wave_speed()).follow()->uri() );

  m_pre_actions->get_child("Zero").configure_option("Fields", cleanup_fields);

  m_post_actions->get_child("ComputeNorm")
      .configure_option("Field", mysolver.fields().get_child( RDM::Tags::residual()).follow()->uri() );

  //----------------------------------------------------------------------------------------

  CFinfo << "[RDM] iterative solve" << CFendl;

  CActionDirector& boundary_conditions =
      access_component( "cpath:../BoundaryConditions" ).as_type<CActionDirector>();

  CActionDirector& domain_discretization =
      access_component( "cpath:../DomainDiscretization" ).as_type<CActionDirector>();

  CAction& synchronize = mysolver.actions().get_child("Synchronize").as_type<CAction>();

  Component& cnorm = m_post_actions->get_child("ComputeNorm");

  // iteration loop

  Uint iter = 1; // iterations start from 1 ( max iter zero will do nothing )
  property("iteration") = iter;


  while( ! stop_condition() ) // non-linear loop
  {
    // (1) the pre actions - cleanup residual, pre-process something, etc

    m_pre_actions->execute();

    // (2) domain discretization

    domain_discretization.execute();

    // (3) apply boundary conditions

    boundary_conditions.execute();

    // (4) update

    m_update->execute();

    // (5) update

    synchronize.execute();

    // (6) the post actions - compute norm, post-process something, etc

    m_post_actions->execute();

    // output convergence info

    /// @todo move current rhs as a property of the iterate or solver components
    {
      Real rhs_norm = cnorm.properties().value<Real>("Norm");
      std::cout << " Iter [" << std::setw(4) << iter << "]"
                << " L2(rhs) [" << std::setw(12) << rhs_norm << "]" << std::endl;

      if ( is_nan(rhs_norm) || is_inf(rhs_norm) )
        throw FailedToConverge(FromHere(),
                               "Solution diverged after "+to_str(iter)+" iterations");
    }

    // raise signal that iteration is done

    /// @todo move this to an Action and/or separate function in base class
    {
      SignalOptions opts; SignalFrame frame;
      opts.add_option< OptionT<Uint> >( "iteration", iter );
      frame = opts.create_frame("iteration_done", uri(), URI());

      Common::Core::instance().event_handler().raise_event( "iteration_done", frame);
    }

    // increment iteration

    property("iteration") = ++iter; // update the iteration number

  }
}

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF
