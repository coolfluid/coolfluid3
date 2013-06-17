// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include "solver/BC.hpp"
#include "solver/Time.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "common/OptionList.hpp"

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

BC::BC( const std::string& name ) : common::Action(name)
{
  options().add("fields",m_fields)
      .link_to(&m_fields)
      .mark_basic()
      .attach_trigger( boost::bind( &BC::create_fields, this) );

  options().add("solution",m_solution)
      .link_to(&m_solution)
      .mark_basic();

  options().add("bdry_fields",m_bdry_fields)
      .link_to(&m_bdry_fields)
      .mark_basic()
      .attach_trigger( boost::bind( &BC::create_fields, this) );

  options().add("bdry_solution",m_bdry_solution)
      .link_to(&m_bdry_solution)
      .mark_basic();

  options().add("bdry_solution_gradient",m_bdry_solution_gradient)
      .link_to(&m_bdry_solution_gradient)
      .mark_basic();

  options().add("time",m_time)
      .link_to(&m_time)
      .mark_basic();

  options().add("regions",m_regions)
      .link_to(&m_regions)
      .mark_basic();
}

BC::~BC() 
{  
}

void BC::create_fields()
{
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
