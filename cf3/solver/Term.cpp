// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include "solver/Term.hpp"
#include "solver/Time.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "common/OptionList.hpp"

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

Term::Term( const std::string& name ) : common::Component(name)
{
  options().add("fields",m_fields)
      .link_to(&m_fields)
      .attach_trigger( boost::bind( &Term::create_fields, this) );
  options().add("solution",m_solution).link_to(&m_solution);

  options().add("bdry_fields",m_bdry_fields)
      .link_to(&m_bdry_fields)
      .attach_trigger( boost::bind( &Term::create_bdry_fields, this) );
  options().add("bdry_solution",m_bdry_solution).link_to(&m_bdry_solution);
  options().add("bdry_solution_gradient",m_bdry_solution).link_to(&m_bdry_solution_gradient);

  options().add("time",m_time).link_to(&m_time);
}

Term::~Term() 
{  
}

//void Term::add_configuration_options(Component& component)
//{
//}

void Term::create_fields()
{
}

void Term::create_bdry_fields()
{
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
