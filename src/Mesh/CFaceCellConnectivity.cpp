// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>
#include <deque>

#include "Common/OptionT.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CLink.hpp"
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CCells.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CFaceCellConnectivity , Component, LibMesh > CFaceCellConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

CFaceCellConnectivity::CFaceCellConnectivity ( const std::string& name ) : 
  Component(name),
  m_nb_faces(0)
{

  m_elements = create_static_component<CUnifiedData<CCells> >("elements");
  m_connectivity = create_static_component<CTable<Uint> >("connectivity_table");
  m_face_nb_in_first_elem = create_static_component<CList<Uint> >("face_number");
  m_is_bdry_face = create_static_component<CList<Uint> >("is_bdry_face");
  m_connectivity->set_row_size(2);
}

////////////////////////////////////////////////////////////////////////////////

void CFaceCellConnectivity::setup(CRegion& region)
{
  m_elements->add_data(find_components_recursively<CCells>(region).as_vector());  
  build_connectivity();
}

////////////////////////////////////////////////////////////////////////////////

void CFaceCellConnectivity::build_connectivity()
{
  
  if (m_elements->size() == 0 )
  {
    CFwarn << "No elements are given to build faces of" << CFendl;
    return;
  }

  m_connectivity->set_row_size(2);

  // declartions
  CTable<Uint>::Buffer f2c = m_connectivity->create_buffer();
  CList<Uint>::Buffer face_number = m_face_nb_in_first_elem->create_buffer();
  CList<Uint>::Buffer is_bdry_face = m_is_bdry_face->create_buffer();
  CNodes& nodes = m_elements->data_components()[0]->nodes(); 
  Uint tot_nb_nodes = nodes.size();
  std::vector < std::vector<Uint> > mapNodeFace(tot_nb_nodes);
  std::vector<Uint> face_nodes;  face_nodes.reserve(100);
  std::vector<Uint> dummy_row(2);
  Uint max_nb_faces(0);

  // calculate max_nb_faces
  boost_foreach ( CCells::Ptr elements, m_elements->data_components() )
  { 
    if (elements->element_type().dimensionality() != elements->element_type().dimension() )
      continue;
    const Uint nb_faces = elements->element_type().nb_faces();
    max_nb_faces += nb_faces * elements->size() ;
  }
  
  // allocate storage if doesn't exist that says if the element is at the boundary of a region
  // ( = not the same as the mesh boundary)
  boost_foreach (CCells::Ptr elements, m_elements->data_components())
  {
    Component::Ptr comp = elements->get_child_ptr("is_bdry");
    if ( is_null( comp ) || is_null(comp->as_ptr< CList<bool> >()) )
    {
      CList<bool>& is_bdry_elem = * elements->create_component< CList<bool> >("is_bdry");

      const Uint nb_elem = elements->size();
      is_bdry_elem.resize(nb_elem);

      for (Uint e=0; e<nb_elem; ++e)
        is_bdry_elem[e] = true;
    }
    cf_assert( elements->get_child_ptr("is_bdry")->as_ptr< CList<bool> >() );
  }

  // Declarations to save frequent allocations in the loop algorithm
  Uint elem_idx = 0;
  Uint nb_inner_faces = 0;
  Uint nb_matched_nodes = 1;
  Uint face_node_idx;
  Uint face_node;
  Uint face;
  Uint connected_face;
  Uint node;
  Uint nb_nodes;
  bool found_face = false;
  CCells::Ptr elem_location_comp;
  Uint elem_location_idx;

  // loop over the element types
  m_nb_faces=0;
  boost_foreach (CCells::Ptr& elements, m_elements->data_components() )
  {
    const Uint nb_faces_in_elem = elements->element_type().nb_faces();
    CList<bool>& is_bdry_elem = *elements->get_child_ptr("is_bdry")->as_ptr< CList<bool> >();

    // loop over the elements of this type
    Uint loc_elem_idx=0;
    boost_foreach(CTable<Uint>::ConstRow elem, elements->connectivity_table().array() ) 
    {
      if ( is_bdry_elem[loc_elem_idx] )
      {
        // loop over the faces in the current element
        for (Uint face_idx = 0; face_idx != nb_faces_in_elem; ++face_idx)
        {
          // construct sets of nodes that make the corresponding face in this element
          nb_nodes = elements->element_type().face_type(face_idx).nb_nodes();
          face_nodes.resize(nb_nodes);
          index_foreach(i,const Uint face_node_idx, elements->element_type().face_connectivity().face_node_range(face_idx))
             face_nodes[i] = elem[face_node_idx];


          // consider the first node belonging to the current face
          // check if you find a face ID shared between all the other
          // nodes building a face
          node = face_nodes[0];
          cf_assert(node<tot_nb_nodes);
          
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
                  f2c.get_row(face)[1]=elem_idx;
                  is_bdry_face.get_row(face)=false;
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
              f2c.get_row(face)[1]=elem_idx;
              is_bdry_face.get_row(face)=false;

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

            face_number.add_row(face_idx);

            // increment the number of faces
            dummy_row[0]=elem_idx;
            f2c.add_row(dummy_row);
            is_bdry_face.add_row(true);
            ++m_nb_faces;
          }
        }
        ++elem_idx;
        ++loc_elem_idx;
      }
    }
  }
  f2c.flush();
  face_number.flush();
  is_bdry_face.flush();

  //CFinfo << "Total nb faces [" << m_nb_faces << "]" << CFendl;
  //CFinfo << "Inner nb faces [" << nb_inner_faces << "]" << CFendl;

  // total number of boundary + partition boundary faces
  //const Uint nb_bdry_plus_partition_faces = m_nb_faces - nb_inner_faces;
  //CFinfo << "Boundary and Partition faces [" << nb_bdry_plus_partition_faces << "]" << CFendl;

  cf_assert(m_nb_faces <= max_nb_faces);
  cf_assert(nb_inner_faces <= max_nb_faces);


  for (Uint f=0; f<m_connectivity->size(); ++f)
  {
    boost_foreach (Uint elem, (*m_connectivity)[f])
    {
      boost::tie(elem_location_comp,elem_location_idx) = m_elements->data_location(elem);
      CList<bool>& is_bdry_elem = *elem_location_comp->get_child_ptr("is_bdry")->as_ptr< CList<bool> >();
      is_bdry_elem[elem_location_idx] = is_bdry_elem[elem_location_idx] || is_bdry_face.get_row(f) ;
    }
  }
  

#if 0
  for (Uint f=0; f<m_connectivity->size(); ++f)
  {
    if ( is_bdry_face[f] )
    {
      bdry_faces.add_row(f2c.get_row(f)[0]);
      bdry_face_number.add_row(face_number.get_row(f));
      f2c.rm_row(f);
      face_number.rm_row(f);
      --m_nb_faces;
    }
  }
  f2c.flush();
  face_number.flush();
#endif
}

////////////////////////////////////////////////////////////////////////////////

CTable<Uint>::ConstRow CFaceCellConnectivity::elements(const Uint unified_face_idx) const
{
  return (*m_connectivity)[unified_face_idx];
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData<CCells>::data_location_type CFaceCellConnectivity::element_location(const Uint unified_elem_idx)
{
  return m_elements->data_location(unified_elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData<CCells>::const_data_location_type CFaceCellConnectivity::element_location(const Uint unified_elem_idx) const
{
  return m_elements->data_location(unified_elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

std::vector<Uint> CFaceCellConnectivity::nodes(const Uint face) const
{
  Uint unified_elem_idx = (*m_connectivity)[face][0];
  CCells::Ptr elem_comp;
  Uint elem_idx;
  boost::tie(elem_comp,elem_idx) = m_elements->data_location(unified_elem_idx);
  std::vector<Uint> nodes(elem_comp->element_type().face_type((*m_face_nb_in_first_elem)[face]).nb_nodes());
  index_foreach (i, Uint node_in_face, elem_comp->element_type().face_connectivity().face_node_range((*m_face_nb_in_first_elem)[face]))
  {
    nodes[i] = elem_comp->connectivity_table()[elem_idx][node_in_face];
  }
  return nodes;
}

////////////////////////////////////////////////////////////////////////////////

void CFaceCellConnectivity::add_elements( CUnifiedData<CCells>::Ptr elements )
{
  m_elements->add_data(find_components(elements->data_links()).as_vector());
}

////////////////////////////////////////////////////////////////////////////////

boost::tuple<Uint,Uint> CFaceCellConnectivity::element_loc_idx(const Uint unified_elem_idx)
{
  return m_elements->data_local_idx(unified_elem_idx);
}


////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
