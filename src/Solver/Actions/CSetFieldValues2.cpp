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
  CLoopOperation(name)
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
  m_field = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(m_field.lock()) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
}

/////////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues2::execute()
{
  CFinfo << "face [" << idx() << "]" << CFendl;

  // m_idx is the index that is set using the function set_loop_idx()
  CField2& field = *m_field.lock();
  CTable<Real>::Row data = field[idx()];
  const Real x = field.coords(idx())[XX];
  //const CF::Real y =  field.coords(idx())[YY];
  
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

