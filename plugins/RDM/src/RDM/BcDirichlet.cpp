// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Log.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CEntities.hpp"

#include "RDM/BcDirichlet.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BcDirichlet, CAction, LibRDM > BcDirichlet_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
BcDirichlet::BcDirichlet ( const std::string& name ) :
  Solver::Actions::CLoopOperation(name)
{
  // options
  m_properties.add_option< OptionURI > ("Field","Field to apply Bc to", URI("cpath:"))->mark_basic();
  m_properties["Field"].as_option().attach_trigger ( boost::bind ( &BcDirichlet::config_field,   this ) );

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &BcDirichlet::trigger_elements,   this ) );

  m_field_view = create_static_component<CFieldView>("field_view");
}

////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::trigger_elements()
{
//  CFinfo << "elements [" << elements().full_path().string() << "]" << CFendl;

//  m_can_start_loop =
      m_field_view->set_elements( elements() );
}

void BcDirichlet::config_field()
{
  URI uri;
  property("Field").put_value(uri);

  Component::Ptr found = look_component(uri);
  if( is_null(found) )
    throw InvalidURI (FromHere(), "URI does not point to a component " + uri.string() );

  m_field = found->as_type<CField2>();
  if ( is_null( m_field.lock() ) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");

  m_field_view->set_field( m_field.lock() );
}

/////////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::execute()
{
//  CFinfo << "face [" << idx() << "]" << CFendl;

  // m_idx is the index that is set using the function set_loop_idx()
  CField2& field = *m_field.lock();
  CTable<Real>::Row data = field[idx()];
  const Real x = field.coords(idx())[XX];
//  const CF::Real y =  field.coords(idx())[XX];

  const Uint row_size = data.size();
  for (Uint i = 0; i != row_size; ++i)
  {
    if (x >= -1.4 && x <= -0.6)
      data[i] = 0.5*(cos(3.141592*(x+1.0)/0.4)+1.0);
    else
      data[i] = 0.0;
  }
//  CFinfo << "x = " << x << CFendl;
//  CFinfo << "data = " << data[0] << CFendl;

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

