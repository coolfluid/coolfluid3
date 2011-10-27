// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/actions/InitFieldConstant.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitFieldConstant, MeshTransformer, mesh::actions::LibActions> InitFieldConstant_Builder;

//////////////////////////////////////////////////////////////////////////////

InitFieldConstant::InitFieldConstant( const std::string& name )
: MeshTransformer(name),
  m_constant(0.)
{

  m_properties["brief"] = std::string("Initialize a field with a constant value");
  std::string desc;
  desc = "  Usage: InitFieldConstant constant \n";
  m_properties["description"] = desc;

  options().add_option(OptionComponent<Field>::create("field", &m_field))
      ->description("Field to initialize")
      ->pretty_name("Field")
      ->mark_basic();

  options().add_option< OptionT<Real> > ("constant", m_constant)
      ->description("Constant applied as initial field")
      ->pretty_name("Constant")
      ->link_to( &m_constant )
      ->mark_basic();
}

/////////////////////////////////////////////////////////////////////////////

std::string InitFieldConstant::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string InitFieldConstant::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" + properties().value<std::string>("description");
}

////////////////////////////////////////////////////////////////////////////////

void InitFieldConstant::execute()
{
  if (m_field.expired())
    throw SetupError(FromHere(), "option [field] was not set in ["+uri().path()+"]");

  Field& field = *m_field.lock();
  for (Uint i=0; i<field.size(); ++i)
    for (Uint j=0; j<field.row_size(); ++j)
      field[i][j] = m_constant;
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
