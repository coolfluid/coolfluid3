// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Solver/LibSolver.hpp"
#include "Solver/CTime.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ComponentBuilder < CTime, Component, LibSolver > CTime_Builder;

////////////////////////////////////////////////////////////////////////////////

CTime::CTime ( const std::string& name  ) :
  Component ( name ),
  m_time(0.),
  m_dt(0.)
{
  m_properties.add_option< OptionT<Real> > ("Time","Current time of the simulation", m_time)->mark_basic();
  m_properties.add_option< OptionT<Real> > ("Time Step","Time Step of simulation", m_dt)->mark_basic();
  m_properties.add_option< OptionT<Real> > ("End Time", "Time at which to finish the simulation", m_time)->mark_basic();

  m_properties["Time"].as_option().link_to( &m_time );
  m_properties["Time Step"].as_option().link_to( &m_dt );
}

////////////////////////////////////////////////////////////////////////////////

CTime::~CTime()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
