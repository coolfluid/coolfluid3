// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Foreach.hpp"
#include "Common/OptionArray.hpp"

#include "Mesh/Actions/CInitSolution.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CSpace.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
    
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CInitSolution, CMeshTransformer, LibActions> CInitSolution_Builder;

//////////////////////////////////////////////////////////////////////////////

CInitSolution::CInitSolution( const std::string& name )
: CMeshTransformer(name)
{
   
  properties()["brief"] = std::string("Initialize a solution");
  std::string desc;
  desc = 
    "  Usage: CInitSolution vectorial function \n";
  properties()["description"] = desc;
  
  m_properties.add_option<
      OptionArrayT<std::string> > ("Functions",
                                   "Math function applied as initial solution (vars x,y,z)",
                                   std::vector<std::string>())
      ->attach_trigger ( boost::bind ( &CInitSolution::config_function, this ) )
      ->mark_basic();
      
  m_function.variables("x,y,z");

}

/////////////////////////////////////////////////////////////////////////////

std::string CInitSolution::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CInitSolution::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CInitSolution::config_function()
{
  m_function.functions( m_properties["Functions"].value<std::vector<std::string> >() );
  m_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void CInitSolution::transform(const CMesh::Ptr& mesh)
{

  m_mesh = mesh;

  CField2& solution_field = find_component_with_tag<CField2>(*mesh,"solution");

  std::vector<Real> vars(3,0.);

  RealVector return_val(solution_field.data().row_size());

  if (solution_field.basis() == CField2::Basis::POINT_BASED)
  {
    const Uint nb_pts = solution_field.size();
    for ( Uint idx=0; idx!=nb_pts; ++idx)
    {      
      CTable<Real>::ConstRow coords = solution_field.coords(idx);
      for (Uint i=0; i<coords.size(); ++i)
        vars[i] = coords[i];
      
      m_function.evaluate(vars,return_val);
      
      CTable<Real>::Row data_row = solution_field[idx];
      for (Uint i=0; i<data_row.size(); ++i)
        data_row[i] = return_val[i];
    }
  }
  else 
  {
    CFieldView solution("solution_view");
    solution.set_field(solution_field);
    RealMatrix coordinates;
    boost_foreach( CElements& elements, find_components_recursively<CElements>(solution_field.topology()) )
    {
      if (solution.set_elements(elements.as_ptr<CEntities>()))
      {
        solution.allocate_coordinates(coordinates);
        RealVector centroid(coordinates.rows());
        
        for (Uint elem_idx = 0; elem_idx<elements.size(); ++elem_idx)
        {
          solution.put_coordinates( coordinates, elem_idx );
          solution.space().shape_function().compute_centroid( coordinates , centroid );
          
          for (Uint i=0; i<centroid.size(); ++i)
            vars[i] = centroid[i];

          m_function.evaluate(vars,return_val);

          CTable<Real>::Row data_row = solution[elem_idx];
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
