// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Solver/CModel.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ObjectProvider < CModel, Component, LibSolver, NB_ARGS_1 >
CModel_Provider ( CModel::type_name() );

////////////////////////////////////////////////////////////////////////////////

CModel::CModel( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CModel::~CModel()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
