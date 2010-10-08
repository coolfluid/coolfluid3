// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/OptionT.hpp"
#include "Common/ObjectProvider.hpp"

#include "CPhysicalModel.hpp"


using namespace CF::Common;

namespace CF {
namespace Solver {

Common::ObjectProvider < CPhysicalModel, Component, LibSolver, NB_ARGS_1 >
CPhysicalModel_Provider ( CPhysicalModel::type_name() );
  
CPhysicalModel::CPhysicalModel(const CName& name) : Component(name)
{
  BUILD_COMPONENT;
}

CPhysicalModel::~CPhysicalModel()
{
}

void CPhysicalModel::defineConfigProperties(PropertyList& options)
{
  options.add_option<OptionT <Uint> >("dimensions", "Dimensionality of the problem, i.e. the number of components for the spatial coordinates", DIM_0D);
  options.add_option<OptionT <Uint> >("dof", "Degrees of freedom", 0u);
}

Uint CPhysicalModel::dimensions() const
{
  return boost::any_cast<Uint>(property("dimensions").value());
}

Uint CPhysicalModel::nb_dof() const
{
  return boost::any_cast<Uint>(property("dof").value());
}

}
}
