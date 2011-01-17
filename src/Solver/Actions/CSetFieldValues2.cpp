// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Log.hpp"
#include "Mesh/CField2.hpp"

#include "Solver/Actions/CSetFieldValues2.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CSetFieldValues2, CLoopOperation, LibActions > CSetFieldValues2_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
CSetFieldValues2::CSetFieldValues2 ( const std::string& name ) : 
  CLoopOperation(name),
  m_field("Field")
{
  // options
  m_properties.add_option< OptionURI > ("Field","Field to set", URI("cpath:"))->mark_basic();
  m_properties["Field"].as_option().attach_trigger ( boost::bind ( &CSetFieldValues2::config_field,   this ) );
}

////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues2::config_field()
{
  URI uri;
  property("Field").put_value(uri);
  CField2::Ptr field = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(field) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
  m_field.link_to(field);
}

/////////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues2::execute()
{
  // m_idx is the index that is set using the function set_loop_idx()
  
  CTable<Real>::Row data = m_field.field()[m_idx];
  const Real x = m_field.field().coords(m_idx)[XX];
  //const CF::Real y =  m_field.field().coords(m_idx)[YY];
  
  const Uint row_size = data.size();
  for (Uint i = 0; i != row_size; ++i)
  {
    if (x >= -1.4 && x <= -0.6)
      data[i] = 0.5*(cos(3.141592*(x+1.0)/0.4)+1.0);
    else
      data[i] = 0.0;
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

