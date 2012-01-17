// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"

#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/UnifiedData.hpp"

#include "solver/actions/CComputeArea.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CComputeArea, CLoopOperation, LibActions > CComputeArea_Builder;

///////////////////////////////////////////////////////////////////////////////////////

CComputeArea::CComputeArea ( const std::string& name ) :
  CLoopOperation(name)
{
  // options
  /// @todo make this option a OptionComponent
  options().add_option(mesh::Tags::area(), URI())
      .supported_protocol(URI::Scheme::CPATH)
      .description("Field to set")
      .pretty_name("Area")
      .mark_basic()
      .attach_trigger ( boost::bind ( &CComputeArea::config_field, this ) )
      .add_tag(mesh::Tags::area());

  options()["elements"].attach_trigger ( boost::bind ( &CComputeArea::trigger_elements,   this ) );
}

////////////////////////////////////////////////////////////////////////////////

void CComputeArea::config_field()
{
  URI uri = options().option(mesh::Tags::area()).value<URI>();
  m_area = Core::instance().root().access_component(uri)->handle<Field>();
}

////////////////////////////////////////////////////////////////////////////////

void CComputeArea::trigger_elements()
{
  m_can_start_loop = m_area->dict().defined_for_entities(elements().handle<Entities>());
  if (m_can_start_loop)
  {
    elements().allocate_coordinates(m_coordinates);
    m_area_field_space = m_area->space(elements()).handle<Space>();
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void CComputeArea::execute()
{
  const Space& space = *m_area_field_space;
  Field& area = *m_area;

  elements().put_coordinates(m_coordinates,idx());
  area[space.indexes_for_element(idx())[0]][0] = elements().element_type().area( m_coordinates );
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

