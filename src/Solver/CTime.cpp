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
  Component ( name )
{
  m_properties.add_option< OptionT<Real> > ("Starting Time","Time of start of simulation", 0.)->mark_basic();
  m_properties.add_option< OptionT<Real> > ("Time Step","Time of start of simulation", 0.)->mark_basic();
  
  m_properties["Starting Time"].as_option().link_to( &m_time );
  m_properties["Time Step"].as_option().link_to( &m_dt );
}

////////////////////////////////////////////////////////////////////////////////

CTime::~CTime()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
