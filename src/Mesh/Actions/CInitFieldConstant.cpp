// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/Actions/CInitFieldConstant.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField2.hpp"
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
   
  properties()["brief"] = std::string("Initialize a field with a constant value");
  std::string desc;
  desc = 
    "  Usage: CInitFieldConstant constant \n";
  properties()["description"] = desc;

  m_properties.add_option(OptionComponent<CField2>::create("Field","Field to initialize",&m_field))
    ->mark_basic();
  
  m_properties.add_option<
      OptionT<Real> > ("Constant","Constant applied as initial field", m_constant)
      ->link_to( &m_constant )
      ->mark_basic();
}

/////////////////////////////////////////////////////////////////////////////

std::string CInitFieldConstant::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CInitFieldConstant::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  

////////////////////////////////////////////////////////////////////////////////

void CInitFieldConstant::execute()
{
  if (m_field.expired())
    throw SetupError(FromHere(), "Field option in ["+full_path().path()+"] was not set");
  m_field.lock()->data() = m_constant;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
