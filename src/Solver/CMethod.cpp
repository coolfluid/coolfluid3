// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Solver/LibSolver.hpp"
#include "Solver/CMethod.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::String;

Common::ComponentBuilder < CMethod, Component, LibSolver > CMethod_Builder;

////////////////////////////////////////////////////////////////////////////////

CMethod::CMethod ( const std::string& name  ) :
  Component ( name )
{
   

  m_properties.add_option< Common::OptionT<bool> >("myBoolMeth", "A boolean value in a CMethod", true);
  m_properties.add_option< Common::OptionT<int> >("fourtyTwo", "An integer value in a CMethod", 42);
  m_properties.add_option< Common::OptionT<CF::Real> >("euler", "Euler number in a CMethod", 2.71);

  this->regist_signal ( "run_operation" , "run an operation", "Run Operation" )->connect ( boost::bind ( &CMethod::run_operation, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

CMethod::~CMethod()
{
}

////////////////////////////////////////////////////////////////////////////////

CMethod& CMethod::operation(const std::string& name)
{
  return *get_child<CMethod>(name);
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
