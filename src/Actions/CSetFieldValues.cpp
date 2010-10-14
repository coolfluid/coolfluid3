// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"
#include "Actions/CSetFieldValues.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"

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
  m_field = look_component_type<CField>(field_path);
}

/////////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues::execute()
{
//  CF_DEBUG_POINT;
//  CFLogVar(m_field->full_path().string());
  
  BOOST_FOREACH(CFieldElements& elements, recursive_range_typed<CFieldElements>(*m_field))
  {
    
    CArray& field_data = elements.data();
    CArray& coordinates = elements.coordinates();
    
    BOOST_FOREACH(const Uint n, elements.node_list().array())
    {
			const CF::Real x = coordinates[n][XX];
			//const CF::Real y = coordinates[n][YY];

			for (Uint i = 0; i < field_data.row_size(); ++i)
			{
				if (x >= -1.4 && x <= -0.6)
					field_data[n][i] = 0.5*(cos(3.141592*(x+1.0)/0.4)+1.0);
				else
					field_data[n][i] = 0.0;
			}
		}
  }
  
  // BOOST_FOREACH(CArray::Ptr field_data, field_data_set)
  //   {
  // 
  //     CArray& coordinates = *field_data->get_child("coordinates")->get_type<CArray>();
  //     
  //     // setting values of fields here
  //     for ( Uint n = 0; n < field_data->size(); ++n)
  //     {
  //       const CF::Real x = coordinates[n][XX];
  //       const CF::Real y = coordinates[n][YY];
  // 
  //       for (Uint i = 0; i < field_data->row_size(); ++i)
  //       {
  //         if (x >= -0.8 && x <= -0.4)
  //           (*field_data)[n][i] = cos(3.141592*(x+0.6)/0.2)+1.0;
  //         else
  //           (*field_data)[n][i] = 0.0;
  //       }
  //       
  //       // CFinfo << "field_data["<<n<<"] = " << (*field_data)[n][0] << CFendl;
  //       
  //     }
  // 
  //   }
}

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

