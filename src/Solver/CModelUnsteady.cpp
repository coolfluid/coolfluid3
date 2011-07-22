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

#include "Math/Consts.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/CTime.hpp"
#include "Solver/Tags.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;

Common::ComponentBuilder < CModelUnsteady, Component, LibSolver > CModelUnsteady_Builder;

////////////////////////////////////////////////////////////////////////////////

struct CModelUnsteady::Implementation
{
  Implementation(Component& component) :
    m_component(component)
  {
  }

  Component& m_component;
  boost::weak_ptr<CTime> m_time;
};

////////////////////////////////////////////////////////////////////////////////

CModelUnsteady::CModelUnsteady( const std::string& name  ) :
  CModel ( name ),
  m_implementation(new Implementation(*this))
{
  m_properties["steady"] = bool(false);

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

CModelUnsteady::~CModelUnsteady() {}


void CModelUnsteady::simulate ()
{
  CModel::simulate();
  time().configure_option("time", time().current_time() );
}


CTime& CModelUnsteady::create_time(const std::string& name)
{
  CTime::Ptr time = create_component_ptr<CTime>(name);
  m_implementation->m_time = time;

  configure_option_recursively(Tags::time(), time->uri());

  return *time;
}


void CModelUnsteady::signal_create_time(SignalArgs node)
{
  create_time();
}


CTime& CModelUnsteady::time()
{
  if(m_implementation->m_time.expired())
    throw SetupError(FromHere(), "Time is not configured for model " + uri().string());

  return *m_implementation->m_time.lock();
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
