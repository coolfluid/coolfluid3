// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
#include <iomanip>

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CField2.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/CTime.hpp"
#include "Solver/Actions/CIterate.hpp"

#include "FVM/OutputIterationInfo.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

  
///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < OutputIterationInfo, CAction, LibFVM > OutputIterationInfo_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
OutputIterationInfo::OutputIterationInfo ( const std::string& name ) : 
  CAction(name)
{
  mark_basic();
  // options

  m_properties.add_option(OptionComponent<CTime>::create("Time","Time tracking component", &m_time))
    ->add_tag("time");

  m_properties.add_option(OptionComponent<CField2>::create("Residual","Residual", &m_residual))
    ->add_tag("residual");
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
  CFinfo << "Iter [" << std::setw(4) << parent()->as_ptr_checked<CIterate>()->iter() << "]";
  CFinfo << "      Time [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << m_time.lock()->time() << "]";
  CFinfo << "      Time Step [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << m_time.lock()->dt() << "]";
  CFinfo << "      L2(rhs) [" << std::setw(12) << rhs_L2 << "]";
  CFinfo << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

