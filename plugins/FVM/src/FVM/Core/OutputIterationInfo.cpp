// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
#include <iomanip>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/CTime.hpp"
#include "Solver/Actions/CIterate.hpp"

#include "FVM/Core/OutputIterationInfo.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < OutputIterationInfo, CAction, LibCore > OutputIterationInfo_Builder;

///////////////////////////////////////////////////////////////////////////////////////

OutputIterationInfo::OutputIterationInfo ( const std::string& name ) :
  CAction(name)
{
  mark_basic();
  // options

  m_options.add_option(OptionComponent<CTime>::create("ctime", &m_time))
      ->description("Time tracking component")
      ->pretty_name("Time");

  m_options.add_option(OptionComponent<CField>::create("residual", &m_residual))
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
  boost_foreach(CTable<Real>::ConstRow rhs , m_residual.lock()->data().array())
    rhs_L2 += rhs[0]*rhs[0];
  rhs_L2 = sqrt(rhs_L2) / m_residual.lock()->data().size();

  // output convergence info
  CFinfo << "Iter [" << std::setw(4) << parent().as_ptr_checked<CIterate>()->iter() << "]";
  CFinfo << "      Time [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << m_time.lock()->current_time() << "]";
  CFinfo << "      Time Step [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << m_time.lock()->dt() << "]";
  CFinfo << "      L2(rhs) [" << std::setw(12) << rhs_L2 << "]";
  CFinfo << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

