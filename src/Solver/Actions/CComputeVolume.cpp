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
#include "Mesh/CEntities.hpp"

#include "Solver/Actions/CComputeVolume.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CComputeVolume, CLoopOperation, LibActions > CComputeVolume_Builder;

///////////////////////////////////////////////////////////////////////////////////////

CComputeVolume::CComputeVolume ( const std::string& name ) :
  CLoopOperation(name)
{
  // options
  /// @todo make this option a OptionComponent
  m_options.add_option(OptionURI::create("volume", URI("cpath:"), URI::Scheme::CPATH))
      ->description("Field to set")
      ->mark_basic()
      ->attach_trigger ( boost::bind ( &CComputeVolume::config_field,   this ) )
      ->add_tag(Mesh::Tags::volume());

  m_options["elements"].attach_trigger ( boost::bind ( &CComputeVolume::trigger_elements,   this ) );

}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::config_field()
{
  URI uri;
  option("Volume").put_value(uri);
  m_volume = Core::instance().root().access_component_ptr(uri)->as_ptr<Field>();
}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::trigger_elements()
{
  m_can_start_loop = m_volume.lock()->elements_lookup().contains(elements());
  if (m_can_start_loop)
  {
    elements().allocate_coordinates(m_coordinates);
    m_volume_field_space = m_volume.lock()->space(elements()).as_ptr<CSpace>();
    m_volume_field_space.lock()->allocate_coordinates(m_coordinates);
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::execute()
{
  CSpace& space = *m_volume_field_space.lock();
  Field& volume = *m_volume.lock();

  m_coordinates = space.compute_coordinates(idx());
  volume[space.indexes_for_element(idx())[0]][0] = elements().element_type().compute_volume( m_coordinates );
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

