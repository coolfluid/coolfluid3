// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////////////
  
CAction::CAction ( const std::string& name ) : 
  Component(name)
{
   this->regist_signal ( "execute" , "Execute the action", "Execute" )->connect ( boost::bind ( &CAction::execute, this ) );
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

