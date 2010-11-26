// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CFactories.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

CFactories::CFactories ( const std::string& name) : Component ( name )
{
  add_tag( type_name() );

  // this type must be registered immedietly on creation,
  // registration could be defered to after the Core has been inialized.
  RegistTypeInfo<CFactories> CBuilder_TypeRegistration();

}

////////////////////////////////////////////////////////////////////////////////

CFactories::~CFactories()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
