// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

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

  m_options.add_option(OptionComponent<CField>::create("field", &m_field))
      ->set_description("Field to initialize")
      ->set_pretty_name("Field")
      ->mark_basic();

  m_options.add_option< OptionArrayT<std::string> > ("functions", std::vector<std::string>())
      ->set_description("Math function applied as initial field (vars x,y,z)")
      ->set_pretty_name("Functions definition")
      ->attach_trigger ( boost::bind ( &CInitFieldFunction::config_function, this ) )
      ->mark_basic();

  m_function.variables("x,y,z");

}

////////////////////////////////////////////////////////////////////////////////

CInitFieldFunction::~CInitFieldFunction()
{
}

/////////////////////////////////////////////////////////////////////////////

void CInitFieldFunction::config_function()
{
  m_function.functions( m_options["functions"].value<std::vector<std::string> >() );
  m_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void CInitFieldFunction::execute()
{
  if (m_field.expired())
    throw SetupError(FromHere(), "Option [field] was not set in ["+uri().path()+"]");

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
    CMultiStateFieldView field_view("field_view");
    field_view.set_field(field);
    RealMatrix coordinates;
    boost_foreach( CElements& elements, find_components_recursively<CElements>(field.topology()) )
    {
      if (field_view.set_elements(elements))
      {
        elements.allocate_coordinates(coordinates);
        RealVector centroid(coordinates.cols());
        cf_assert(centroid.size() < 4);

        for (Uint elem_idx = 0; elem_idx<elements.size(); ++elem_idx)
        {
          elements.put_coordinates( coordinates, elem_idx );


          CMultiStateFieldView::View data_rows = field_view[elem_idx];
          /// for each state of the field shape function
          for (Uint iState=0; iState<field_view.space().nb_states(); ++iState)
          {
            /// get its local coordinates from the SPACE shape_function
            RealVector local_coords = field_view.space().shape_function().local_coordinates().row(iState);
            /// get the physical coordinates through the GEOMETRIC shape function (from element_type)
            RealVector physical_coords = elements.element_type().shape_function().value(local_coords)*coordinates;
            /// evaluate the function using the physical coordinates
            for (Uint d=0; d<physical_coords.size(); ++d)
              vars[d] = physical_coords[d];
            m_function.evaluate(vars,return_val);
            /// put the return values in the field
            for (Uint i=0; i<data_rows[iState].size(); ++i)
            {
              data_rows[iState][i] = return_val[i];
            }
          }
        }

      }

    }

  }

}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
