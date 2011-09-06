// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"

#include "Mesh/Field.hpp"
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
  /// @todo make this option a OptionComponent
  m_options.add_option(OptionURI::create(Mesh::Tags::area(), URI("cpath:"), URI::Scheme::CPATH) )
      ->description("Field to set")
      ->pretty_name("Area")
      ->mark_basic()
      ->attach_trigger ( boost::bind ( &CComputeArea::config_field, this ) )
      ->add_tag(Mesh::Tags::area());

  m_options["elements"].attach_trigger ( boost::bind ( &CComputeArea::trigger_elements,   this ) );
}

////////////////////////////////////////////////////////////////////////////////

void CComputeArea::config_field()
{
  URI uri;
  option(Mesh::Tags::area()).put_value(uri);
  m_area = Core::instance().root().access_component_ptr(uri)->as_ptr<Field>();
}

////////////////////////////////////////////////////////////////////////////////

void CComputeArea::trigger_elements()
{
  m_can_start_loop = m_area.lock()->elements_lookup().contains(elements());
  if (m_can_start_loop)
  {
    elements().allocate_coordinates(m_coordinates);
    m_area_field_space = m_area.lock()->space(elements()).as_ptr<CSpace>();
    m_area_field_space.lock()->allocate_coordinates(m_coordinates);
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void CComputeArea::execute()
{
  CSpace& space = *m_area_field_space.lock();
  Field& area = *m_area.lock();

  m_coordinates = space.compute_coordinates(idx());
  area[space.indexes_for_element(idx())[0]][0] = elements().element_type().area( m_coordinates );
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

