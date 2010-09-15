// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CFlexTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CFlexTable, Component, MeshLib, NB_ARGS_1 >
CFlexTable_Provider ( CFlexTable::type_name() );

////////////////////////////////////////////////////////////////////////////////

CFlexTable::CFlexTable ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
