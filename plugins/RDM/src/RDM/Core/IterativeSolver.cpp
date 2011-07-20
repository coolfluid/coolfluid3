// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Solver/Actions/CSynchronizeFields.hpp"
#include "Solver/Actions/CCriterionMaxIterations.hpp"
#include "Solver/Actions/CComputeLNorm.hpp"

#include "RDM/Core/Solver.hpp"
#include "RDM/Core/Cleanup.hpp"

#include "IterativeSolver.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Solver::Actions;
using namespace CF::Mesh;

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

  Cleanup::Ptr cleanup  = create_component_ptr<Cleanup>("Zero");

  m_pre_actions->append( cleanup );

  CCriterionMaxIterations& maxiter =
      create_component<CCriterionMaxIterations>( "MaxIterations" );

  CComputeLNorm& cnorm =
      create_component<CComputeLNorm>( "ComputeNorm" );

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

//  m_compute_norm->configure_option("Field", m_residual.lock()->uri());

//  std::vector<URI> cleanup_fields;
//  cleanup_fields.push_back( m_residual.lock()->uri() );
//  cleanup_fields.push_back( m_wave_speed.lock()->uri() );
//  m_cleanup->configure_option("Fields", cleanup_fields);

  CFinfo << "[RDM] iterative solve" << CFendl;


  CActionDirector& boundary_conditions =
      access_component( "cpath:../BoundaryConditions" ).as_type<CActionDirector>();

  CActionDirector& domain_discretization =
      access_component( "cpath:../DomainDiscretization" ).as_type<CActionDirector>();

  CAction& synchronize =
      solver().as_type< Core::Solver >()
      .actions().get_child("Synchronize").as_type<CAction>();

  /// @todo this configuration sould be in constructor but does not work there

  get_child("MaxIterations").configure_option( "iterator", this->uri() );

  // iteration loop

  Uint k = 1; // iterations start from 1 ( max iter zero will do nothing )
  property("iteration") = k;


  while( ! stop_condition() ) // non-linear loop
  {

    // print iteration

    CFinfo << "    K-iter [" << k << "]" << CFendl;

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

    // increment iteration

    property("iteration") = ++k; // update the iteration number

  }
}

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF
