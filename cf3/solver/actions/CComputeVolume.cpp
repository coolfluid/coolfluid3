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
#include "mesh/Entities.hpp"
#include "mesh/UnifiedData.hpp"

#include "solver/actions/CComputeVolume.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CComputeVolume, CLoopOperation, LibActions > CComputeVolume_Builder;

///////////////////////////////////////////////////////////////////////////////////////

CComputeVolume::CComputeVolume ( const std::string& name ) :
  CLoopOperation(name)
{
  // options
  /// @todo make this option a OptionComponent
  options().add_option("volume", URI())
      .supported_protocol(URI::Scheme::CPATH)
      .description("Field to set")
      .mark_basic()
      .attach_trigger ( boost::bind ( &CComputeVolume::config_field,   this ) )
      .add_tag(mesh::Tags::volume());

  options()["elements"].attach_trigger ( boost::bind ( &CComputeVolume::trigger_elements,   this ) );

}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::config_field()
{
  URI uri = options().option("volume").value<URI>();
  m_volume = Core::instance().root().access_component(uri)->handle<Field>();
}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::trigger_elements()
{
  m_can_start_loop = m_volume->dict().defined_for_entities(elements().handle<Entities>());
  if (m_can_start_loop)
  {
    elements().geometry_space().allocate_coordinates(m_coordinates);
    m_volume_field_space = m_volume->space(elements()).handle<Space>();
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::execute()
{
  const Space& space = *m_volume_field_space;
  Field& volume = *m_volume;

  elements().geometry_space().put_coordinates(m_coordinates,idx());
  volume[space.indexes_for_element(idx())[0]][0] = elements().element_type().volume( m_coordinates );
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

