// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Mesh/CField2.hpp"
#include "Math/MatrixTypes.hpp"

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

}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::config_field()
{
  URI uri;
  property("Volumes").put_value(uri);
  m_field = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(m_field.lock()) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
}

////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::trigger_elements()
{
  if ( is_null(m_field.lock()) )
    throw ValueNotFound (FromHere(),"Volumes option must be configured first");
  m_index = elements().get_child(m_field.lock()->name())->get_child<CTable<Uint> >("index");
}

/////////////////////////////////////////////////////////////////////////////////////

void CComputeVolume::execute()
{
  // idx() is the index that is set using the function set_loop_idx() or configuration LoopIndex

  CElements& elems = elements();
  CField2& volumes = volume_field();
  
  CTable<Uint>::ConstRow& field_indexes = (*m_index.lock())[idx()];
  
  boost_foreach (Uint field_idx, field_indexes)
    volumes[field_idx][0] = elems.element_type().compute_volume( elems.element_coordinates(idx()) );
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

