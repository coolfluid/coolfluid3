// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CList.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

//////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CList<bool>, Component, LibMesh, NB_ARGS_1 >
CList_bool_Provider ( CList<bool>::type_name() );

Common::ObjectProvider < CList<Uint>, Component, LibMesh, NB_ARGS_1 >
CList_Uint_Provider ( CList<Uint>::type_name() );

Common::ObjectProvider < CList<int>, Component, LibMesh, NB_ARGS_1 >
CList_int_Provider ( CList<int>::type_name() );

Common::ObjectProvider < CList<Real>, Component, LibMesh, NB_ARGS_1 >
CList_Real_Provider ( CList<Real>::type_name() );

Common::ObjectProvider < CList<std::string>, Component, LibMesh, NB_ARGS_1 >
CList_string_Provider ( CList<std::string>::type_name() );

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
