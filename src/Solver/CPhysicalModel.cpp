// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/OptionT.hpp"
#include "Common/CBuilder.hpp"

#include "CPhysicalModel.hpp"

using namespace CF::Common;

namespace CF {
namespace Solver {

Common::ComponentBuilder < CPhysicalModel, Component, LibSolver > CPhysicalModel_Builder;
  
////////////////////////////////////////////////////////////////////////////////

CPhysicalModel::CPhysicalModel(const std::string& name) : Component(name),
m_dim(0u),
m_nbdofs(0u)
{
  tag_component(this); define_config_properties(); define_signals();

  m_properties["Dimensions"].as_option().attach_trigger ( boost::bind ( &CPhysicalModel::trigger_dimensionality, this ) );
  m_properties["DOFs"].as_option().attach_trigger ( boost::bind ( &CPhysicalModel::trigger_nbdofs, this ) );

}

CPhysicalModel::~CPhysicalModel()
{
}

void CPhysicalModel::define_config_properties()
{
  m_properties.add_option<OptionT <Uint> >("Dimensions", "Dimensionality of the problem, i.e. the number of components for the spatial coordinates", 0u);
  m_properties.add_option<OptionT <Uint> >("DOFs", "Degrees of freedom", 0u);

  m_properties["Dimensions"].as_option().mark_basic();
  m_properties["DOFs"].as_option().mark_basic();
}

void CPhysicalModel::trigger_dimensionality()
{
  m_dim = boost::any_cast<Uint>(property("Dimensions").value());
}

void CPhysicalModel::trigger_nbdofs()
{
  m_nbdofs = boost::any_cast<Uint>(property("DOFs").value());
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
