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

#include "Solver/Actions/CPeriodicWriteMesh.hpp"
#include "Solver/Actions/CSynchronizeFields.hpp"
#include "Solver/Actions/CCriterionMaxIterations.hpp"
#include "Solver/Actions/CComputeLNorm.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/Reset.hpp"

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

  // properties

  m_properties.add_property( "iteration", Uint(0) );

  // static components

  m_pre_actions  = create_static_component_ptr<CActionDirector>("PreActions");

  m_update = create_static_component_ptr<CActionDirector>("Update");

  m_post_actions = create_static_component_ptr<CActionDirector>("PostActions");

  // dynamic components

  create_component<CCriterionMaxIterations>( "MaxIterations" );

  CComputeLNorm& cnorm = post_actions().create_component<CComputeLNorm>( "ComputeNorm" );
  post_actions().append( cnorm );

  CPeriodicWriteMesh& cwriter = post_actions().create_component<CPeriodicWriteMesh>( "PeriodicWriter" );
  post_actions().append( cwriter );

  cnorm.configure_option("Scale", true);
  cnorm.configure_option("Order", 2u);
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

  /// @todo this configuration sould be in constructor but does not work there

  configure_option_recursively( "iterator", this->uri() );

  // access components (out of loop)

  CActionDirector& boundary_conditions =
      access_component( "cpath:../BoundaryConditions" ).as_type<CActionDirector>();

  CActionDirector& domain_discretization =
      access_component( "cpath:../DomainDiscretization" ).as_type<CActionDirector>();

  CAction& synchronize = mysolver.actions().get_child("Synchronize").as_type<CAction>();

  Component& cnorm = post_actions().get_child("ComputeNorm");
  cnorm.configure_option("Field", mysolver.fields().get_child( RDM::Tags::residual() ).follow()->uri() );

  // iteration loop

  Uint iter = 1; // iterations start from 1 ( max iter zero will do nothing )
  property("iteration") = iter;


  while( ! stop_condition() ) // non-linear loop
  {
    // (1) the pre actions - cleanup residual, pre-process something, etc

    pre_actions().execute();

    // (2) domain discretization

    domain_discretization.execute();

    // (3) apply boundary conditions

    boundary_conditions.execute();

    // (4) update

    update().execute();

    // (5) update

    synchronize.execute();

    // (6) the post actions - compute norm, post-process something, etc

    post_actions().execute();

    // output convergence info

    /// @todo move current rhs as a prpoerty of the iterate or solver components
    if( Comm::PE::instance().rank() == 0 )
    {
      Real rhs_norm = cnorm.properties().value<Real>("Norm");
      CFinfo << "iter ["    << std::setw(4)  << iter << "]"
             << "L2(rhs) [" << std::setw(12) << rhs_norm << "]" << CFendl;

      if ( is_nan(rhs_norm) || is_inf(rhs_norm) )
        throw FailedToConverge(FromHere(),
                               "Solution diverged after "+to_str(iter)+" iterations");
    }

    // raise signal that iteration is done

    raise_iteration_done();

    // increment iteration

    property("iteration") = ++iter; // update the iteration number

  }
}

void IterativeSolver::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add_option< OptionT<Uint> >( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());

  Common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
