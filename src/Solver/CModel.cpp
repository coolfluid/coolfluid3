// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Solver/CModel.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ComponentBuilder < CModel, Component, LibSolver > CModel_Builder;

////////////////////////////////////////////////////////////////////////////////

CModel::CModel( const std::string& name  ) :
  Component ( name )
{
  tag_component(this);
}

////////////////////////////////////////////////////////////////////////////////

CModel::~CModel()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
