// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include "common/Log.hpp"
#include "common/CBuilder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StreamHelpers.hpp"
#include "common/StringConversion.hpp"

#include "Mesh/Actions/CBuildFaceNormals.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CNodeFaceCellConnectivity.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/Field.hpp"

#include "Math/Functions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
namespace Actions {

  using namespace common;
  using namespace Math::Functions;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CBuildFaceNormals, CMeshTransformer, LibActions> CBuildFaceNormals_Builder;

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
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CBuildFaceNormals::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CBuildFaceNormals::execute()
{

  CMesh& mesh = *m_mesh.lock();

  const Uint dimension = mesh.geometry().coordinates().row_size();

  FieldGroup& faces_P0 = mesh.create_space_and_field_group("faces_P0",FieldGroup::Basis::FACE_BASED,"CF.Mesh.LagrangeP0");
  Field& face_normals = faces_P0.create_field(Mesh::Tags::normal(),std::string(Mesh::Tags::normal())+"[vector]");
  face_normals.add_tag(Mesh::Tags::normal());

  Component::Ptr component;
  Uint cell_idx(0);
  boost_foreach( CEntities& faces, face_normals.entities_range() )
  {
    CFaceCellConnectivity::Ptr face2cell_ptr = find_component_ptr<CFaceCellConnectivity>(faces);
    if (is_not_null(face2cell_ptr))
    {
      CFaceCellConnectivity& face2cell = *face2cell_ptr;
      CTable<Uint>& face_nb = face2cell.face_number();
      RealMatrix face_coordinates(faces.element_type().nb_nodes(),faces.element_type().dimension());
      RealVector normal(faces.element_type().dimension());
      for (Uint face=0; face<face2cell.size(); ++face)
      {
        // The normal will be outward to the first connected element
        boost::tie(component,cell_idx) = face2cell.lookup().location(face2cell.connectivity()[face][FIRST]);
        CCells& cells = component->as_type<CCells>();
        CConnectivity::ConstRow cell_nodes = cells.node_connectivity()[cell_idx];
        Uint i(0);
        boost_foreach(Uint node_id, cells.element_type().faces().nodes_range(face_nb[face][0]) )
        {
          Uint j(0);
          boost_foreach(const Real& coord, mesh.geometry().coordinates()[cell_nodes[node_id]])
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
          face_normals[face_normals.indexes_for_element(faces,face)[0]][XX]=normal[XX];
        }
        else
        {
          faces.element_type().compute_normal(face_coordinates,normal);
          Uint field_index = face_normals.indexes_for_element(faces,face)[0];
          cf3_assert(field_index    < face_normals.size()    );
          cf3_assert(normal.size() == face_normals.row_size());
          for (Uint i=0; i<normal.size(); ++i)
            face_normals[field_index][i]=normal[i];

        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // cf3
