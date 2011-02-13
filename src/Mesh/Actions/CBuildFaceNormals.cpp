// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/String/Conversion.hpp"

#include "Mesh/Actions/CBuildFaceNormals.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CNodeFaceCellConnectivity.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSpace.hpp"
#include "Math/MathFunctions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
  using namespace Math::MathFunctions;
  using namespace Common::String;
    
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBuildFaceNormals, CMeshTransformer, LibActions> CBuildFaceNormals_Builder;

//////////////////////////////////////////////////////////////////////////////

CBuildFaceNormals::CBuildFaceNormals( const std::string& name )
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

std::string CBuildFaceNormals::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CBuildFaceNormals::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CBuildFaceNormals::transform(const CMesh::Ptr& mesh)
{

  m_mesh = mesh;

  const Uint dimension = mesh->nodes().coordinates().row_size();
  CField2& face_normal_field = mesh->create_field2("face_normal","FaceBased","face_normal["+to_str(dimension)+"]");
  face_normal_field.add_tag("face_normal");
  CFieldView face_normal("face_normal_view");
  face_normal.set_field(face_normal_field);
  boost_foreach( CEntities& faces, find_components_recursively_with_tag<CEntities>(m_mesh->topology(),"face_entity") )
  {
    face_normal.set_elements(faces);
    
    CFaceCellConnectivity::Ptr face2cell_ptr = find_component_ptr<CFaceCellConnectivity>(faces);
    if (is_not_null(face2cell_ptr))
    {
      CFaceCellConnectivity& face2cell = *face2cell_ptr;
      RealMatrix coordinates(faces.element_type().nb_nodes(),faces.element_type().dimension());
      RealVector normal(faces.element_type().dimension());
      for (Uint face=0; face<face2cell.size(); ++face)
      {
        // The normal will be outward to the first connected element
        Uint elem = face2cell.elements(face)[0];
        face_normal.put_coordinates(coordinates,face);
        face_normal.space().shape_function().compute_normal(coordinates,normal);
        for (Uint i=0; i<normal.size(); ++i)
          face_normal[face][i] = normal[i];
      }
    }
  }

}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
