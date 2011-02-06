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
  m_properties.add_option< OptionURI > ("Volumes","Field to set", URI("cpath:"))->mark_basic();
  m_properties["Volumes" ].as_option().attach_trigger ( boost::bind ( &CComputeVolume::config_field,   this ) );
  
  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &CComputeVolume::trigger_elements,   this ) );

  m_volume = create_static_component<CScalarFieldView>("volume_view");
}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::config_field()
{
  URI uri;
  property("Volumes").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
  m_volume->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::trigger_elements()
{
  m_volume->set_elements(elements());
  m_volume->allocate_coordinates(m_coordinates);
  m_can_start_loop = m_volume->field().exists_for_entities(elements());
}

/////////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::execute()
{
  // idx() is the index that is set using the function set_loop_idx() or configuration LoopIndex
  
  m_volume->put_coordinates(m_coordinates,idx());
  Real vol = m_volume->space().shape_function().compute_volume( m_coordinates );

  Uint state_idx = 0;
  Uint var_idx = 0;

  // 3 ways to access the field through field views:
    
  // 1) as simple scalar field --> only 1 index needed (this is already default here)
  CScalarFieldView& volume = *m_volume;
  volume[idx()] = vol;
  
  // 2) as simple field --> extra index for multiple variables per field 
  CFieldView& view = m_volume->as<CFieldView>();
  view[idx()][var_idx] = vol;

  // 3) as complex field --> extra index for the case with multiple states per element
  CMultiStateFieldView& multi_state_view = m_volume->as<CMultiStateFieldView>();
  multi_state_view[idx()][state_idx][var_idx] = vol;

}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

