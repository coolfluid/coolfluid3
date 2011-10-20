// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/CBuilder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "Mesh/Actions/CInitFieldConstant.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/CSpace.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
namespace Actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CInitFieldConstant, CMeshTransformer, LibActions> CInitFieldConstant_Builder;

//////////////////////////////////////////////////////////////////////////////

CInitFieldConstant::CInitFieldConstant( const std::string& name )
: CMeshTransformer(name),
  m_constant(0.)
{

  m_properties["brief"] = std::string("Initialize a field with a constant value");
  std::string desc;
  desc = "  Usage: CInitFieldConstant constant \n";
  m_properties["description"] = desc;

  m_options.add_option(OptionComponent<Field>::create("field", &m_field))
      ->description("Field to initialize")
      ->pretty_name("Field")
      ->mark_basic();

  m_options.add_option< OptionT<Real> > ("constant", m_constant)
      ->description("Constant applied as initial field")
      ->pretty_name("Constant")
      ->link_to( &m_constant )
      ->mark_basic();
}

/////////////////////////////////////////////////////////////////////////////

std::string CInitFieldConstant::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CInitFieldConstant::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" + properties().value<std::string>("description");
}

////////////////////////////////////////////////////////////////////////////////

void CInitFieldConstant::execute()
{
  if (m_field.expired())
    throw SetupError(FromHere(), "option [field] was not set in ["+uri().path()+"]");

  Field& field = *m_field.lock();
  for (Uint i=0; i<field.size(); ++i)
    for (Uint j=0; j<field.row_size(); ++j)
      field[i][j] = m_constant;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // cf3
