// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"

#include "Mesh/CFieldView.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"

#include "Solver/Actions/CComputeArea.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CComputeArea, CLoopOperation, LibActions > CComputeArea_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
CComputeArea::CComputeArea ( const std::string& name ) : 
  CLoopOperation(name)
{
  // options
  m_properties.add_option< OptionURI > ("Area","Field to set", URI("cpath:"))->mark_basic();
  m_properties["Area" ].as_option().attach_trigger ( boost::bind ( &CComputeArea::config_field,   this ) );
  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &CComputeArea::trigger_elements,   this ) );

  m_area = create_static_component<CScalarFieldView>("area_view");
}

////////////////////////////////////////////////////////////////////////////////

void CComputeArea::config_field()
{
  URI uri;
  property("Area").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
  m_area->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void CComputeArea::trigger_elements()
{
  m_area->set_elements(elements());
  m_area->allocate_coordinates(m_coordinates);
}

/////////////////////////////////////////////////////////////////////////////////////

void CComputeArea::execute()
{
  CScalarFieldView& area = *m_area;

  area.put_coordinates(m_coordinates,idx());
  area[idx()] = area.space().shape_function().compute_area( m_coordinates );;
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

