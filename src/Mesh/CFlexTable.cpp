// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CFlexTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CFlexTable, Component, LibMesh, NB_ARGS_1 >
CFlexTable_Provider ( CFlexTable::type_name() );

////////////////////////////////////////////////////////////////////////////////

CFlexTable::CFlexTable ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
  m_idx.resize(1);
  m_idx[0]=0;
}

// std::ostream& operator<<(std::ostream& os, const CFlexTable::ConstRow& row)
// {
//   print_vector(os, row);
//   return os;
// }

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
