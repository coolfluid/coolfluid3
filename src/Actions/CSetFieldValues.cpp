// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"
#include "Actions/CSetFieldValues.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CSetFieldValues, CAction, LibActions, NB_ARGS_1 > CSetFieldValuesProvider( "CSetFieldValues" );

///////////////////////////////////////////////////////////////////////////////////////
  
void CSetFieldValues::defineConfigProperties( Common::PropertyList& options )
{
  options.add_option< OptionT<URI> > ("Field","Field URI to output", URI("cpath://"))->mark_basic();
}

///////////////////////////////////////////////////////////////////////////////////////

CSetFieldValues::CSetFieldValues ( const CName& name ) : 
  CAction(name)
{
  BUILD_COMPONENT;
  m_property_list["Field"].as_option().attach_trigger ( boost::bind ( &CSetFieldValues::trigger_Field,   this ) );  
}

/////////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues::trigger_Field()
{
  CPath field_path (property("Field").value<URI>());
  CFdebug << "field_path = " << field_path.string() << CFendl;
  m_field = look_component_type<CField>(field_path);
}

/////////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues::execute()
{
  BOOST_FOREACH(CArray& field_data, recursive_filtered_range_typed<CArray>(*m_field,IsComponentTag("field_data")))
  {

    CArray& coordinates = *field_data.get_child("coordinates")->get_type<CArray>();
    
    // setting values of fields here
    for ( Uint n = 0; n < field_data.size(); ++n)
      for (Uint i = 0; i < field_row.size(); ++i)
      {
        const CF::Real x = coordinates[n][XX];
        const CF::Real y = coordinates[n][YY];
        field_data[n][i] = cos ( x*y );
      }

  }
}

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

