// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/EventHandler.hpp"
#include "Common/OptionT.hpp"
#include "Common/CActionDirector.hpp"
#include "Common/FindComponents.hpp"

#include "Solver/Actions/CCriterion.hpp"
#include "Solver/Actions/CCriterionMaxIterations.hpp"

#include "SFDM/IterativeSolver.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Solver::Actions;

namespace CF {
namespace SFDM {


///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < IterativeSolver, CAction, LibSFDM > IterativeSolver_Builder;

///////////////////////////////////////////////////////////////////////////////////////

IterativeSolver::IterativeSolver ( const std::string& name ) :
  CAction(name)
{
  mark_basic();

  // properties

  m_properties.add_property( "iteration", Uint(0) );

  // static components

  m_pre_update = create_static_component_ptr<CActionDirector>("PreUpdate");

  m_update = create_static_component_ptr<CActionDirector>("Update");

  m_post_update = create_static_component_ptr<CActionDirector>("PostUpdate");

  // dynamic components
  CCriterionMaxIterations& maxiter =
    create_component<CCriterionMaxIterations>( "MaxIterations" );

}

///////////////////////////////////////////////////////////////////////////////////////

bool IterativeSolver::stop_condition()
{
  Uint nb_criteria = 0;
  bool finish = false;
  boost_foreach(CCriterion& stop_criterion, find_components<CCriterion>(*this))
  {
    finish |= stop_criterion();
    ++nb_criteria;
  }
  if (nb_criteria == 0)
  {
    CFwarn << "No stop criteria available in [" << uri().string() << "]... exiting loop" << CFendl;
    return true; // stop
  }
  return finish;
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::execute()
{

  /// @todo these configurations sould be in constructor but does not work there
  ///       becasue uri() is undefined on the constructor ( component is still free )
  configure_option_recursively( "iterator", this->uri() );

  // iteration loop=
  Uint iter = 1; // iterations start from 1 ( max iter zero will do nothing )
  property("iteration") = iter;

  while( ! stop_condition() ) // non-linear loop
  {
//    CFinfo << "pre-update" << CFendl;
    pre_update().execute();
//    CFinfo << "update" << CFendl;
    update().execute();
//    CFinfo << "post-update" << CFendl;
    post_update().execute();

    // raise signal that iteration is done
    raise_iteration_done();

    // increment iteration
    property("iteration") = ++iter; // update the iteration number
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void IterativeSolver::raise_iteration_done()
{
  SignalOptions opts;
  const Uint iter = properties().value<Uint>("iteration");
  opts.add_option< OptionT<Uint> >( "iteration", iter );
  SignalFrame frame = opts.create_frame("iteration_done", uri(), URI());

  Common::Core::instance().event_handler().raise_event( "iteration_done", frame);
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF
