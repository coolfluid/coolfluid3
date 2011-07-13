// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"

#include "Solver/CSolver.hpp"

namespace CF {
namespace Solver {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CSolver::CSolver ( const std::string& name  ) :
  Component ( name )
{
  mark_basic();

  // properties

  m_properties["brief"] = std::string("Solver");
  m_properties["description"] = std::string("");

  // options

  m_options.add_option< OptionURI > ("domain", URI("cpath:../Domain"))
      ->set_description("Domain to solve")
      ->set_pretty_name("Domain");

  // signals

  regist_signal ("solve" ,
                 "Solves",
                 "Solve" )
      ->signal->connect ( boost::bind ( &CSolver::signal_solve, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

CSolver::~CSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

void CSolver::signal_solve ( Common::SignalArgs& node )
{
  this->solve(); // dispatch to the virtual function
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
