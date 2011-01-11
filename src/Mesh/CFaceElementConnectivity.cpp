// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>
#include <deque>

#include "Common/ComponentPredicates.hpp"

#include "Common/CLink.hpp"
#include "Common/Log.hpp"

#include "Mesh/CFaceElementConnectivity.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CNodes.hpp"

#include "Mesh/CNodeElementConnectivity.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CFaceElementConnectivity::CFaceElementConnectivity ( const std::string& name ) : 
  Component(name)
{
  m_elements = create_static_component<CUnifiedData<CElements> >("elements");
  m_connectivity = create_static_component<CDynTable<Uint> >("connectivity_table");
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void CFaceElementConnectivity::build_connectivity()
{
  /// variable that will count nb_faces;
  m_nb_faces=0;
  
  
  // create a table to store the connectivity element-face ID locally to this processor
  // and store the connectivity in MeshData
  // elem_to_face->resize(m_elements->size());

  if (m_elements->size() == 0 )
  {
    CFwarn << "No elements are given to build faces of" << CFendl;
    return;
  }
  CNodes& nodes = m_elements->data_components()[0]->nodes(); 
  // allocate a table mapping node-face ID
  std::vector < std::deque<Uint> > mapNodeFace(nodes.size());

  // atomic number to indicate the maximum possible number
  // of nodes in a face
  // allows to avoid frequent reallocations of the vector nodesInFace
  std::vector<Uint> face_nodes;
  face_nodes.reserve(100);
  
  std::vector<Uint> empty_connectivity_row(0);
  
  Uint max_nb_faces = 0;
  
  boost_foreach ( CElements::Ptr elements, m_elements->data_components() )
  {
    //end_idx = begin_idx + elements->size();
    const Uint nb_faces = elements->element_type().nb_faces();
    max_nb_faces += nb_faces * elements->size() ;
    
    //for (Uint idx=begin_idx; idx<end_idx; ++idx)
    //{
    //  elem_to_face->set_row_size(idx,nb_faces);
    //}
    
    //begin_idx = end_idx;
  }

  // loop over the elements and construct faceIDs
  Uint elem_idx = 0;
  Uint nb_inner_faces = 0;
  Uint nb_matched_nodes = 1;
  Uint face_node_idx;
  Uint face_node;
  Uint face;
  Uint connected_face;
  Uint node;
  bool found_face = false;

  // during the first big loop the following is done:
  // 1. set the element-faces connectivity
  // 2. set the geometric entity type IDs of each face
  // 3. select which are the boundary faces and which are internal ones

  // loop over the types
  boost_foreach ( CElements::Ptr& elements, m_elements->data_components() )
  {
    /// @todo for now all geoents have same geometric and solution polyorder
    const Uint nb_faces_in_elem = elements->element_type().nb_faces();

    // loop over the elements of this type
    boost_foreach( CTable<Uint>::ConstRow elem, elements->connectivity_table().array() )
    {
      // loop over the faces in the current element
      for (Uint face_idx = 0; face_idx != nb_faces_in_elem; ++face_idx)
      {
        // construct sets of nodes that make the corresponding face in this element
        const Uint nb_nodes = elements->element_type().face_type(face_idx).nb_nodes();
        face_nodes.resize(nb_nodes);
        index_foreach(i,const Uint face_node_idx, elements->element_type().face_connectivity().face_node_range(face_idx))
           face_nodes[i] = elem[face_node_idx];


        // consider the first node belonging to the current face
        // check if you find a face ID shared between all the other
        // nodes building a face
        node = face_nodes[0];

        // number of faceIDs referencing node
        // loop over all the faceIDs referenced by the first node to see if
        // all the other nodes reference the same face
        found_face = false;
        // search for matching face if they are registered to the node yet
        boost_foreach( face, mapNodeFace[node] )
        {
          nb_matched_nodes = 1;
          if (nb_nodes > 1) 
          {
            for (face_node_idx=1; face_node_idx!=nb_nodes; ++face_node_idx)
            {
              boost_foreach (connected_face, mapNodeFace[face_nodes[face_node_idx]])
              {
                if (connected_face == face)
                {
                  ++nb_matched_nodes;
                  break;
                }
              }

              
              if (nb_matched_nodes == nb_nodes) // face matches the nodes
              {
                // the corresponding face already exists, meaning
                // that the face is an internal one, shared by two elements
                // here you set the second element (==state) neighbor of the face
                found_face = true;
                //(*m_connectivity)[elemID][iFace] = currFaceID;
                (*m_connectivity)[face].push_back(elem_idx);

                // since it has two neighbor cells,
                // this face is surely NOT a boundary face
                // m_isBFace[currFaceID] = false;

                // increment number of inner faces (they always have 2 states)
                ++nb_inner_faces;
                break;
              }
            }
          }
          else // this only applies to the 1D case 
          {
            // the corresponding faceID already exists, meaning
            // that the face is an internal one, shared by two elements
            // here you set the second element (==state) neighbor of the face
            found_face = true;
            //(*elem_to_face)[elemID][iFace] = currFaceID;
            (*m_connectivity)[face].push_back(elem_idx);

            // since it has two neighbor cells,
            // this face is surely NOT a boundary face
            //m_isBFace[currFaceID] = false;

            // increment number of inner faces (they always have 2 states)
            ++nb_inner_faces;
            break;
          }
        }
        if (found_face == false) 
        {
          // a new face has been found
          // add the ID of the new face in the corresponding nodes
          // referencing it
          boost_foreach (face_node, face_nodes)
          {
            mapNodeFace[face_node].push_back(m_nb_faces);
          }

          // store the geometric entity type for the current face
          //m_geoTypeIDs[m_nbFaces] = faceGeoTypeID[iFace];

          //(*elem_to_face)[elemID][iFace] = m_nbFaces;
          //nbFaceNodes.push_back(nbNodesPerFace);

          // increment the number of faces
          cf_assert(m_connectivity->size() == m_nb_faces);
          m_connectivity->array().push_back(empty_connectivity_row);
          (*m_connectivity)[m_nb_faces].reserve(2);
          (*m_connectivity)[m_nb_faces].push_back(elem_idx);
          ++m_nb_faces;
        }
      }
      ++elem_idx;
    }
  }

  CFinfo << "Total nb faces [" << m_nb_faces << "]" << CFendl;
  CFinfo << "Inner nb faces [" << nb_inner_faces << "]" << CFendl;

  // total number of boundary + partition boundary faces
  const Uint nb_bdry_plus_partition_faces = m_nb_faces - nb_inner_faces;
  CFinfo << "Boundary and Partition faces [" << nb_bdry_plus_partition_faces << "]" << CFendl;

  cf_assert(m_nb_faces <= max_nb_faces);
  cf_assert(nb_inner_faces <= max_nb_faces);

  // 
  // m_nbInFacesNodes.resize(nbInnerFaces);
  // m_nbBFacesNodes.resize(nbBPlusPartitionFaces);
  // 
  // // set the number of nodes in faces
  // CFuint iBFace = 0;
  // CFuint iInFace = 0;
  // for (CFuint i = 0; i < nbFaceNodes.size(); ++i) {
  //   if (!m_isBFace[i]) {
  //     m_nbInFacesNodes[iInFace++] = nbFaceNodes[i];
  //   }
  //   else {
  //     m_nbBFacesNodes[iBFace++] = nbFaceNodes[i];
  //   }
  // }

}

////////////////////////////////////////////////////////////////////////////////

CDynTable<Uint>::ConstRow CFaceElementConnectivity::elements(const Uint unified_face_idx) const
{
  return (*m_connectivity)[unified_face_idx];
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData<CElements>::data_location_type CFaceElementConnectivity::element_location(const Uint unified_elem_idx)
{
  return m_elements->data_location(unified_elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData<CElements>::const_data_location_type CFaceElementConnectivity::element_location(const Uint unified_elem_idx) const
{
  return m_elements->data_location(unified_elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
