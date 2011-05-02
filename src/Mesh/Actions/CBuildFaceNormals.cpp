// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/StringConversion.hpp"

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
#include "Mesh/CMesh.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Math/MathFunctions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;
  using namespace Math::MathFunctions;

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

void CBuildFaceNormals::execute()
{

  CMesh& mesh = *m_mesh.lock();
  
  const Uint dimension = mesh.nodes().coordinates().row_size();
  CField& face_normal_field = mesh.create_field(Mesh::Tags::normal(),CField::Basis::FACE_BASED,"P0","face_normal["+to_str(dimension)+"]");
  face_normal_field.add_tag(Mesh::Tags::normal());
  CFieldView face_normal("face_normal_view");
  face_normal.set_field(face_normal_field);
  Component::Ptr component;
  Uint cell_idx(0);
  boost_foreach( CEntities& faces, find_components_recursively_with_tag<CEntities>(mesh.topology(),Mesh::Tags::face_entity()) )
  {
    face_normal.set_elements(faces);

    CFaceCellConnectivity::Ptr face2cell_ptr = find_component_ptr<CFaceCellConnectivity>(faces);
    if (is_not_null(face2cell_ptr))
    {
      CFaceCellConnectivity& face2cell = *face2cell_ptr;
      CList<Uint>& face_nb = face2cell.get_child("face_number").as_type<CList<Uint> >();
      RealMatrix face_coordinates(faces.element_type().nb_nodes(),faces.element_type().dimension());
      RealVector normal(faces.element_type().dimension());
      for (Uint face=0; face<face2cell.size(); ++face)
      {
        // The normal will be outward to the first connected element
        boost::tie(component,cell_idx) = face2cell.lookup().location(face2cell.connectivity()[face][FIRST]);
        CCells& cells = component->as_type<CCells>();
        CConnectivity::ConstRow cell_nodes = cells.node_connectivity()[cell_idx];
        Uint i(0);
        boost_foreach(Uint node_id, cells.element_type().face_connectivity().face_node_range(face_nb[face]) )
        {
          Uint j(0);
          boost_foreach(const Real& coord, mesh.nodes().coordinates()[cell_nodes[node_id]])
          {
            face_coordinates(i,j) = coord;
            ++j;
          } 
          ++i;
        }
        
        if (faces.element_type().dimensionality() == 0) // cannot compute normal from element_type
        {
          RealVector cell_centroid(1);
          cells.element_type().compute_centroid(cells.get_coordinates(cell_idx),cell_centroid);
          RealVector normal(1);
          normal = face_coordinates.row(0) - cell_centroid;
          normal.normalize();
          face_normal[face][XX]=normal[XX];
        }
        else
        {
          face_normal.elements().element_type().compute_normal(face_coordinates,normal);
          for (Uint i=0; i<normal.size(); ++i)
            face_normal[face][i] = normal[i];
        }
      }      
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
