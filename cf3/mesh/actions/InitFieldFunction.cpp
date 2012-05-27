// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "mesh/actions/InitFieldFunction.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitFieldFunction, MeshTransformer, mesh::actions::LibActions> InitFieldFunction_Builder;

//////////////////////////////////////////////////////////////////////////////

InitFieldFunction::InitFieldFunction( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Initialize a field");
  std::string desc;
  desc =
    "  Usage: InitFieldFunction vectorial function \n";
  properties()["description"] = desc;

  options().add("field", m_field)
      .description("Field to initialize")
      .pretty_name("Field")
      .link_to(&m_field)
      .mark_basic();

  options().add("functions", std::vector<std::string>())
      .description("math function applied as initial field (vars x,y,z)")
      .pretty_name("Functions definition")
      .attach_trigger ( boost::bind ( &InitFieldFunction::config_function, this ) )
      .mark_basic();

  m_function.variables("x,y,z");

}

////////////////////////////////////////////////////////////////////////////////

InitFieldFunction::~InitFieldFunction()
{
}

/////////////////////////////////////////////////////////////////////////////

void InitFieldFunction::config_function()
{
  m_function.functions( options()["functions"].value<std::vector<std::string> >() );
  m_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void InitFieldFunction::execute()
{
  if (is_null(m_field))
    throw SetupError(FromHere(), "Option [field] was not set in ["+uri().path()+"]");

  Field& field = *m_field;

  std::vector<Real> vars(3,0.);

  RealVector return_val(field.row_size());

  if (field.continuous())
  {
    const Uint nb_pts = field.size();
    Field& coordinates = field.coordinates();
    for ( Uint idx=0; idx!=nb_pts; ++idx)
    {
      Field::ConstRow coords = coordinates[idx];
      for (Uint i=0; i<coords.size(); ++i)
        vars[i] = coords[i];

      m_function.evaluate(vars,return_val);

      Field::Row field_row = field[idx];
      for (Uint i=0; i<field_row.size(); ++i)
        field_row[i] = return_val[i];
    }
  }
  else
  {
    boost_foreach( const Handle<Entities>& elements_handle, field.entities_range() )
    {
      Entities& elements = *elements_handle;
      const Space& space = field.space(elements);
      RealMatrix coordinates;
      space.allocate_coordinates(coordinates);
      const Connectivity& field_connectivity = space.connectivity();
      for (Uint elem_idx = 0; elem_idx<elements.size(); ++elem_idx)
      {
        coordinates = space.compute_coordinates(elem_idx);
        /// for each state of the field shape function
        for (Uint iState=0; iState<space.shape_function().nb_nodes(); ++iState)
        {
          /// evaluate the function using the physical coordinates
          for (Uint d=0; d<coordinates.cols(); ++d)
            vars[d] = coordinates.row(iState)[d];
          m_function.evaluate(vars,return_val);
          /// put the return values in the field
          for (Uint i=0; i<field.row_size(); ++i)
            field[field_connectivity[elem_idx][iState]][i] = return_val[i];
        }
      }
    }
  }

}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
