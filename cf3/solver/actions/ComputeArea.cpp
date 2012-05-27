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
#include "mesh/Connectivity.hpp"

#include "solver/actions/ComputeArea.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeArea, LoopOperation, LibActions > ComputeArea_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeArea::ComputeArea ( const std::string& name ) :
  LoopOperation(name)
{
  // options
  /// @todo make this option a OptionComponent
  options().add(mesh::Tags::area(), URI())
      .supported_protocol(URI::Scheme::CPATH)
      .description("Field to set")
      .pretty_name("Area")
      .mark_basic()
      .attach_trigger ( boost::bind ( &ComputeArea::config_field, this ) )
      .add_tag(mesh::Tags::area());

  options()["elements"].attach_trigger ( boost::bind ( &ComputeArea::trigger_elements,   this ) );
}

////////////////////////////////////////////////////////////////////////////////

void ComputeArea::config_field()
{
  URI uri = options().option(mesh::Tags::area()).value<URI>();
  m_area = Core::instance().root().access_component(uri)->handle<Field>();
}

////////////////////////////////////////////////////////////////////////////////

void ComputeArea::trigger_elements()
{
  m_can_start_loop = m_area->dict().defined_for_entities(elements().handle<Entities>());
  if (m_can_start_loop)
  {
    elements().geometry_space().allocate_coordinates(m_coordinates);
    m_area_field_space = m_area->space(elements()).handle<Space>();
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeArea::execute()
{
  const Space& space = *m_area_field_space;
  Field& area = *m_area;

  elements().geometry_space().put_coordinates(m_coordinates,idx());
  area[space.connectivity()[idx()][0]][0] = elements().element_type().area( m_coordinates );
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

