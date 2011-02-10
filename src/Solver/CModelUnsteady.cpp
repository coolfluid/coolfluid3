// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/CField2.hpp"
#include "Mesh/CMesh.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/CIterativeSolver.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;

Common::ComponentBuilder < CModelUnsteady, Component, LibSolver > CModelUnsteady_Builder;

////////////////////////////////////////////////////////////////////////////////

CModelUnsteady::CModelUnsteady( const std::string& name  ) :
  CModel ( name ),
  m_time(),
  m_max_iter(10000)
{
  m_time = create_static_component<CTime>("Time");

  properties()["steady"] = bool(false);
  
  properties()["brief"] = std::string("Unsteady simulator object");
  std::string description =
  "This object handles unsteady time accurate simulations.\n"
  "The simulator consists of some specific components:\n"
  " - \"domain\" which specifies 1 or more geometries used in the simulation.\n"
  " - \"time\" which holds track of time steps and simulation time.\n"
  " - \"physics\" which define the physics of the problem, equations, ...\n"
  " - \"iterative solver\" which will advance the solution in time\n"
  "   The iterative solver delegates space discretization to a \"discretization method\"";
  properties()["description"] = description;
  
  
  properties().add_option< OptionT<Uint> >("Max Iterations","Maximal number of iterations",m_max_iter)
    ->link_to(&m_max_iter);

}

////////////////////////////////////////////////////////////////////////////////

CModelUnsteady::~CModelUnsteady()
{
}

////////////////////////////////////////////////////////////////////////////////

void CModelUnsteady::simulate ()
{
  CFinfo << "\n" << name() << ": start simulation" << CFendl;

  const Real tf  = m_time->property("End Time").value<Real>();

  // loop over time
  Uint iter(0);
  Real tol=1e-12;
  while( m_time->time() < tf-tol && iter != m_max_iter)
  {
    // reset time step to user-specified
    m_time->dt() = m_time->property("Time Step").value<Real>();
    
    // compute which dt to take
    if( m_time->time() + m_time->dt() > tf )
      m_time->dt() = tf - m_time->time();

    // call all (non-linear) iterative solvers to solve this dt step
    boost_foreach(CIterativeSolver& is, find_components<CIterativeSolver>(*this))
      is.solve();

    m_time->time() += m_time->dt();
    
    CFinfo << "Iter [" << std::setw(4) << ++iter << "]";
    CFinfo << "      Time [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << m_time->time() << "]";
    CFinfo << "      Time Step [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << m_time->dt() << "]";
    CFinfo << "      To Do [" << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << tf-m_time->time() << "]";
    CFinfo << CFendl;
  }
  m_time->configure_property("Time",m_time->time());
  CFinfo << name() << ": end simulation\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
