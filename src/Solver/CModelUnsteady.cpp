// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/CSolver.hpp"

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
  m_time = create_static_component_ptr<CTime>("Time");

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

}

////////////////////////////////////////////////////////////////////////////////

CModelUnsteady::~CModelUnsteady()
{
}

////////////////////////////////////////////////////////////////////////////////

void CModelUnsteady::simulate ()
{
  CFinfo << "\n" << name() << ": start simulation" << CFendl;

  // call all (non-linear) iterative solvers to solve this dt step
  boost_foreach(CSolver& is, find_components<CSolver>(*this))
    is.solve();

  m_time->configure_property("time",m_time->time());
  CFinfo << name() << ": end simulation\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
