// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
#include <iomanip>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/Foreach.hpp"

#include "mesh/CField.hpp"
#include "common/Table.hpp"

#include "solver/FlowSolver.hpp"
#include "solver/CTime.hpp"
#include "solver/Actions/CIterate.hpp"

#include "SFDM/OutputIterationInfo.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::Actions;

namespace cf3 {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < OutputIterationInfo, Action, LibSFDM > OutputIterationInfo_Builder;

///////////////////////////////////////////////////////////////////////////////////////

OutputIterationInfo::OutputIterationInfo ( const std::string& name ) :
  solver::Action(name)
{
  mark_basic();

  // options
  m_options.add_option(OptionComponent<CField>::create(FlowSolver::Tags::residual(), &m_residual))
    ->description("Residual")
    ->pretty_name("Residual");
}

////////////////////////////////////////////////////////////////////////////////

void OutputIterationInfo::execute()
{
  if (m_residual.expired())     throw SetupError(FromHere(), "Residual field was not set");
  if (m_time.expired())         throw SetupError(FromHere(), "Time component was not set");

  // compute norm
  Real rhs_L2=0;
  boost_foreach(Table<Real>::ConstRow rhs , m_residual.lock()->data().array())
    rhs_L2 += rhs[0]*rhs[0];
  rhs_L2 = sqrt(rhs_L2) / m_residual.lock()->data().size();

  // output convergence info
  CFinfo << "Iter [" << std::setw(4) << time().iter() << "]";
  CFinfo << "      Time [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << time().time() << "]";
  CFinfo << "      Time Step [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << time().dt() << "]";
  CFinfo << "      L2(rhs) [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << rhs_L2 << "]";
  CFinfo << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

