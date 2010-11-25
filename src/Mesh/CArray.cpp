// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CArray, Component, LibMesh >
CArray_Builder ( CArray::type_name() );

////////////////////////////////////////////////////////////////////////////////

CArray::CArray ( const std::string& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}
	
////////////////////////////////////////////////////////////////////////////////

void CArray::define_config_properties ( Common::PropertyList& options )
{
}
	
////////////////////////////////////////////////////////////////////////////////
	
std::ostream& operator<<(std::ostream& os, const CArray::ConstRow& row)
{
  print_vector(os, row);
  return os;
}


////////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF
