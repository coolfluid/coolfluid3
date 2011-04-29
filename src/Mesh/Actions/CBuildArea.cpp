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

#include "Mesh/Actions/CBuildArea.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CMesh.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
    
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBuildArea, CMeshTransformer, LibActions> CBuildArea_Builder;

//////////////////////////////////////////////////////////////////////////////

CBuildArea::CBuildArea( const std::string& name )
: CMeshTransformer(name)
{
   
  properties()["brief"] = std::string("Print information of the mesh");
  std::string desc;
  desc = 
  "  Usage: Info \n\n"
  "          Information given: internal mesh hierarchy,\n"
  "      element distribution for each region, and element type"; 
  properties()["description"] = desc;
}

/////////////////////////////////////////////////////////////////////////////

std::string CBuildArea::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CBuildArea::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CBuildArea::execute()
{

  CMesh& mesh = *m_mesh.lock();

  CField& area_field = mesh.create_field2("area","FaceBased");
  area_field.add_tag(Mesh::Tags::area());
  CScalarFieldView area("area_view");
  area.set_field(area_field);

  boost_foreach(CEntities& elements, find_components_recursively_with_tag<CEntities>(mesh.topology(),Mesh::Tags::face_entity()) )    
  {
    area.set_elements(elements.as_ptr<CEntities>());
    
    RealMatrix coordinates;  area.allocate_coordinates(coordinates);
    
    for (Uint cell_idx = 0; cell_idx<elements.size(); ++cell_idx)
    {
      elements.put_coordinates( coordinates, cell_idx );
      area[cell_idx] = area.elements().element_type().compute_area( coordinates );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
