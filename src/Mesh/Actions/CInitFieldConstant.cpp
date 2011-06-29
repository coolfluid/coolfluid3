// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/Actions/CInitFieldConstant.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSpace.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CInitFieldConstant, CMeshTransformer, LibActions> CInitFieldConstant_Builder;

//////////////////////////////////////////////////////////////////////////////

CInitFieldConstant::CInitFieldConstant( const std::string& name )
: CMeshTransformer(name),
  m_constant(0.)
{

  m_properties["brief"] = std::string("Initialize a field with a constant value");
  std::string desc;
  desc =
    "  Usage: CInitFieldConstant constant \n";
  m_properties["description"] = desc;

  m_options.add_option(OptionComponent<CField>::create("field","Field","Field to initialize",&m_field))
    ->mark_basic();

  m_options.add_option<
      OptionT<Real> > ("constant","Constant","Constant applied as initial field", m_constant)
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
  m_field.lock()->data() = m_constant;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
