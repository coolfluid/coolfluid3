// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/CIterativeSolver.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ComponentBuilder < CModelUnsteady, Component, LibSolver > CModelUnsteady_Builder;

////////////////////////////////////////////////////////////////////////////////

CModelUnsteady::CModelUnsteady( const std::string& name  ) :
  CModel ( name ),
  m_time()
{
   m_time = create_static_component<CTime>("Time");

   properties()["steady"] = bool(false);
}

////////////////////////////////////////////////////////////////////////////////

CModelUnsteady::~CModelUnsteady()
{
}

////////////////////////////////////////////////////////////////////////////////

void CModelUnsteady::simulate ()
{  
  const Real ti  = m_time->time();
  const Real tf  = m_time->property("End Time").value<Real>();

  Real dt  = m_time->dt();
  Real ct  = ti;

  // loop over time
  while( ct < tf )
  {
    // compute which dt to take
    if( ct + dt > tf )
    {
      dt = tf - ct;
      m_time->dt() = dt;
    }

    // call all (non-linear) iterative solvers
    // to solve this dt step
    boost_foreach(CIterativeSolver& is, find_components<CIterativeSolver>(*this))
      is.solve();

    ct += dt; // update time

  }
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
