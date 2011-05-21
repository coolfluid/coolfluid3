// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"

#include "Mesh/CFieldView.hpp"
#include "Mesh/CField.hpp"
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
  m_properties.add_option(OptionURI::create(Mesh::Tags::area(),"Area","Field to set", URI("cpath:"), URI::Scheme::CPATH) )
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &CComputeArea::config_field,   this ) )
    ->add_tag(Mesh::Tags::area());

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &CComputeArea::trigger_elements,   this ) );

  m_area = create_static_component_ptr<CScalarFieldView>("area_view");
}

////////////////////////////////////////////////////////////////////////////////

void CComputeArea::config_field()
{
  URI uri;
  property(Mesh::Tags::area()).put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_area->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void CComputeArea::trigger_elements()
{
  m_can_start_loop = m_area->set_elements(elements());
  if (m_can_start_loop)
    elements().allocate_coordinates(m_coordinates);
}

/////////////////////////////////////////////////////////////////////////////////////

void CComputeArea::execute()
{
  CScalarFieldView& area = *m_area;

  elements().put_coordinates(m_coordinates,idx());
  area[idx()] = elements().element_type().compute_area( m_coordinates );;
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

