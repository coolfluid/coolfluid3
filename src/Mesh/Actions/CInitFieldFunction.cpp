// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/Actions/CInitFieldFunction.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSpace.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
    
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CInitFieldFunction, CMeshTransformer, LibActions> CInitFieldFunction_Builder;

//////////////////////////////////////////////////////////////////////////////

CInitFieldFunction::CInitFieldFunction( const std::string& name )
: CMeshTransformer(name)
{
   
  properties()["brief"] = std::string("Initialize a field");
  std::string desc;
  desc = 
    "  Usage: CInitFieldFunction vectorial function \n";
  properties()["description"] = desc;

  m_properties.add_option(OptionComponent<CField>::create("Field","Field to initialize",&m_field))
    ->mark_basic();
  
  m_properties.add_option<
      OptionArrayT<std::string> > ("Functions",
                                   "Math function applied as initial field (vars x,y,z)",
                                   std::vector<std::string>())
      ->attach_trigger ( boost::bind ( &CInitFieldFunction::config_function, this ) )
      ->mark_basic();

  m_function.variables("x,y,z");

}

/////////////////////////////////////////////////////////////////////////////

std::string CInitFieldFunction::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CInitFieldFunction::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CInitFieldFunction::config_function()
{
  m_function.functions( m_properties["Functions"].value<std::vector<std::string> >() );
  m_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void CInitFieldFunction::execute()
{
  if (m_field.expired())
    throw SetupError(FromHere(), "Field option in ["+full_path().path()+"] was not set");

  CField& field = *m_field.lock();

  std::vector<Real> vars(3,0.);

  RealVector return_val(field.data().row_size());

  if (field.basis() == CField::Basis::POINT_BASED)
  {
    const Uint nb_pts = field.size();
    for ( Uint idx=0; idx!=nb_pts; ++idx)
    {      
      CTable<Real>::ConstRow coords = field.coords(idx);
      for (Uint i=0; i<coords.size(); ++i)
        vars[i] = coords[i];
      
      m_function.evaluate(vars,return_val);
      
      CTable<Real>::Row data_row = field[idx];
      for (Uint i=0; i<data_row.size(); ++i)
        data_row[i] = return_val[i];
    }
  }
  else 
  {
    CFieldView field_view("field_view");
    field_view.set_field(field);
    RealMatrix coordinates;
    boost_foreach( CElements& elements, find_components_recursively<CElements>(field.topology()) )
    {
      if (field_view.set_elements(elements))
      {
        field_view.allocate_coordinates(coordinates);
        RealVector centroid(coordinates.rows());
        
        for (Uint elem_idx = 0; elem_idx<elements.size(); ++elem_idx)
        {
          field_view.put_coordinates( coordinates, elem_idx );
          field_view.space().shape_function().compute_centroid( coordinates , centroid );
          
          for (Uint i=0; i<centroid.size(); ++i)
            vars[i] = centroid[i];

          m_function.evaluate(vars,return_val);

          CTable<Real>::Row data_row = field_view[elem_idx];
          for (Uint i=0; i<data_row.size(); ++i)
            data_row[i] = return_val[i];
        }

      }

    }
    
  }
  
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
