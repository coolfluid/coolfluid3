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
  m_options.add_option(OptionURI::create("Volume","Field to set", URI("cpath:"),URI::Scheme::CPATH))
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &CComputeVolume::config_field,   this ) )
    ->add_tag(Mesh::Tags::volume());

  m_options["Elements"].attach_trigger ( boost::bind ( &CComputeVolume::trigger_elements,   this ) );

  m_volume = create_static_component_ptr<CScalarFieldView>("volume_view");
}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::config_field()
{
  URI uri;
  option("Volume").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_volume->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::trigger_elements()
{
  m_can_start_loop = m_volume->set_elements(elements());
  if (m_can_start_loop)
    elements().allocate_coordinates(m_coordinates);
}

/////////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::execute()
{
  // idx() is the index that is set using the function set_loop_idx() or configuration LoopIndex

  elements().put_coordinates(m_coordinates,idx());
  Real vol = elements().element_type().compute_volume( m_coordinates );

  Uint state_idx = 0;
  Uint var_idx = 0;

  // 3 ways to access the field through field views:

  // 1) as simple scalar field --> only 1 index needed (this is already default here)
  CScalarFieldView& volume = *m_volume;
  volume[idx()] = vol;

  // 2) as simple field --> extra index for multiple variables per field
  CFieldView& view = m_volume->as_type<CFieldView>();
  view[idx()][var_idx] = vol;

  // // 3) as complex field --> extra index for the case with multiple states per element
  // CMultiStateFieldView& multi_state_view = m_volume->as_type<CMultiStateFieldView>();
  // multi_state_view[idx()][state_idx][var_idx] = vol;

}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

