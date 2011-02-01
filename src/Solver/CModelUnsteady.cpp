// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/ComponentPredicates.hpp"

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

  Real dt  = m_time->property("Time Step").value<Real>();
  Real ct  = ti;

  CFLogVar(dt);
  CFLogVar(ct);
  CFLogVar(tf);

  // initial condition
  CField2::Ptr solution = find_component_ptr_recursively_with_name<CField2>(*this,"solution");
  if (is_not_null(solution))
    solution->data() = 0.;
  
  // loop over time
  while( ct < tf )
  {
    // compute which dt to take
    if( ct + dt > tf )
    {
      dt = tf - ct;
      m_time->dt() = dt;
    }
    
    //CFLogVar(dt);
    
    // call all (non-linear) iterative solvers
    // to solve this dt step
    boost_foreach(CIterativeSolver& is, find_components<CIterativeSolver>(*this))
      is.solve();

    ct += dt; // update time
    CFinfo << "Time [" << ct << "]" << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
