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
#include "Math/Consts.hpp"

#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CCells.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CFaceCellConnectivity , Component, LibMesh > CFaceCellConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

CFaceCellConnectivity::CFaceCellConnectivity ( const std::string& name ) :
  Component(name),
  m_nb_faces(0),
  m_face_building_algorithm(false)
{

  options().add_option< OptionT<bool> >("face_building_algorithm", m_face_building_algorithm)
      ->link_to(&m_face_building_algorithm)
      ->set_description("Improves efficiency for face building algorithm");

  m_used_components = create_static_component_ptr<CGroup>("used_components");
  m_connectivity = create_static_component_ptr<CTable<Uint> >(Mesh::Tags::connectivity_table());
  m_face_nb_in_elem = create_static_component_ptr<CTable<Uint> >("face_number");
  m_is_bdry_face = create_static_component_ptr<CList<bool> >("is_bdry_face");
  m_connectivity->set_row_size(2);
  m_face_nb_in_elem->set_row_size(2);
}

////////////////////////////////////////////////////////////////////////////////

void CFaceCellConnectivity::setup(CRegion& region)
{
  boost_foreach( CCells& cells,  find_components_recursively<CCells>(region) )
    add_used(cells);

  build_connectivity();
}

////////////////////////////////////////////////////////////////////////////////

std::vector<Component::Ptr> CFaceCellConnectivity::used()
{
  std::vector<Component::Ptr> vec;
  boost_foreach( CLink& link, find_components<CLink>(*m_used_components) )
  {
    vec.push_back(link.follow());
  }
  return vec;
}

////////////////////////////////////////////////////////////////////////////////

void CFaceCellConnectivity::add_used (const Component& used_comp)
{
  bool found = false;
  std::vector<Component::Ptr> used_components = used();
  boost_foreach( Component::Ptr comp, used_components )
  {
    if (comp == used_comp.follow())
    {
      found = true;
      break;
    }
  }
  if (found == false)
    m_used_components->create_component_ptr<CLink>("used_component["+to_str(used_components.size())+"]")->link_to(used_comp);

  m_mesh_elements = find_parent_component<CMesh>(used_comp).elements().as_non_const()->as_ptr<CMeshElements>();
}

////////////////////////////////////////////////////////////////////////////////

void CFaceCellConnectivity::build_connectivity()
{

  if (used().size() == 0 )
  {
    CFwarn << "No elements are given to build faces of" << CFendl;
    return;
  }

  cf_assert(is_not_null(m_mesh_elements));
  // sanity check
  //CFinfo << "building face_cell connectivity using " << CFendl;
  boost_foreach(Component::Ptr cells, used() )
  {
    //CFinfo << "  " << cells->uri().path() << CFendl;
    cf_assert_desc("Must call CMesh::elements().update() to add the elements ["+cells->uri().path()+"] in the elements registry",
      m_mesh_elements->contains(*cells));
  }

  // declartions
  m_connectivity->resize(0);
  CTable<Uint>::Buffer f2c = m_connectivity->create_buffer();
  CTable<Uint>::Buffer face_number = m_face_nb_in_elem->create_buffer();
  CList<bool>::Buffer is_bdry_face = m_is_bdry_face->create_buffer();
  CNodes& nodes = find_parent_component<CMesh>(*used()[0]).nodes();
  Uint tot_nb_nodes = nodes.size();
  std::vector < std::vector<Uint> > mapNodeFace(tot_nb_nodes);
  std::vector<Uint> face_nodes;  face_nodes.reserve(100);
  std::vector<Uint> dummy_row(2, Math::Consts::Uint_max());
  Uint max_nb_faces(0);

  // calculate max_nb_faces
  boost_foreach ( Component::Ptr elements_comp, used() )
  {
    CElements& elements = elements_comp->as_type<CElements>();
    if (elements.element_type().dimensionality() != elements.element_type().dimension() )
      continue;
    const Uint nb_faces = elements.element_type().nb_faces();
    max_nb_faces += nb_faces * elements.size() ;
  }

  if (m_face_building_algorithm)
  {
    // allocate storage if doesn't exist that says if the element is at the boundary of a region
    // ( = not the same as the mesh boundary)
    boost_foreach (Component::Ptr elements_comp, used())
    {
      CElements& elements = elements_comp->as_type<CElements>();
      Component::Ptr comp = elements.get_child_ptr("is_bdry");
      if ( is_null( comp ) || is_null(comp->as_ptr< CList<bool> >()) )
      {
        CList<bool>& is_bdry_elem = * elements.create_component_ptr< CList<bool> >("is_bdry");

        const Uint nb_elem = elements.size();
        is_bdry_elem.resize(nb_elem);

        for (Uint e=0; e<nb_elem; ++e)
          is_bdry_elem[e] = true;
      }
      cf_assert( elements.get_child_ptr("is_bdry")->as_ptr< CList<bool> >() );
    }
  }

  // Declarations to save frequent allocations in the loop algorithm
  Uint nb_inner_faces = 0;
  Uint nb_matched_nodes = 1;
  Uint face_node_idx;
  Uint face_node;
  Uint face;
  Uint connected_face;
  Uint node;
  Uint nb_nodes;
  bool found_face = false;
  Component::Ptr elem_location_comp;
  Uint elem_location_idx;

  // loop over the element types
  m_nb_faces=0;
  boost_foreach (Component::Ptr elements_comp, used() )
  {
    CElements& elements = elements_comp->as_type<CElements>();
    const Uint nb_faces_in_elem = elements.element_type().nb_faces();

    CList<bool>::Ptr is_bdry_elem;

    if (m_face_building_algorithm)
      is_bdry_elem = elements.get_child_ptr("is_bdry")->as_ptr< CList<bool> >();

    // loop over the elements of this type
    Uint loc_elem_idx=0;
    boost_foreach(CTable<Uint>::ConstRow elem, elements.node_connectivity().array() )
    {
      if ( is_not_null(is_bdry_elem) )
        if ( (*is_bdry_elem)[loc_elem_idx] == false )
          continue;

      Uint mesh_elements_idx = m_mesh_elements->unified_idx(elements,loc_elem_idx);


      cf_assert(lookup().location(mesh_elements_idx).get<0>() == elements_comp);
      cf_assert(lookup().location(mesh_elements_idx).get<1>() == loc_elem_idx);

      // loop over the faces in the current element
      for (Uint face_idx = 0; face_idx != nb_faces_in_elem; ++face_idx)
      {
        // construct sets of nodes that make the corresponding face in this element
        nb_nodes = elements.element_type().face_type(face_idx).nb_nodes();
        face_nodes.resize(nb_nodes);
        Uint i(0);
        boost_foreach(const Uint face_node_idx, elements.element_type().face_connectivity().face_node_range(face_idx))
            face_nodes[i++] = elem[face_node_idx];


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
                f2c.get_row(face)[1]=mesh_elements_idx;
                face_number.get_row(face)[1]=face_idx;
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
            f2c.get_row(face)[1]=mesh_elements_idx;
            face_number.get_row(face)[1]=face_idx;
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

          // increment the number of faces
          dummy_row[0]=mesh_elements_idx;
          f2c.add_row(dummy_row);

          dummy_row[0]=face_idx;
          face_number.add_row(dummy_row);

          is_bdry_face.add_row(true);
          ++m_nb_faces;
        }
      }
      ++loc_elem_idx;
    } // end foreach element
  } // end foreach elements component

  f2c.flush();
  face_number.flush();
  is_bdry_face.flush();

  // CFinfo << "Total nb faces [" << m_nb_faces << "]" << CFendl;
  // CFinfo << "Inner nb faces [" << nb_inner_faces << "]" << CFendl;

  // total number of boundary + partition boundary faces
  //const Uint nb_bdry_plus_partition_faces = m_nb_faces - nb_inner_faces;
  //CFinfo << "Boundary and Partition faces [" << nb_bdry_plus_partition_faces << "]" << CFendl;

  cf_assert(m_nb_faces <= max_nb_faces);
  cf_assert(nb_inner_faces <= max_nb_faces);


  cf_assert(m_nb_faces == m_connectivity->size());

  if (m_face_building_algorithm)
  {
    for (Uint f=0; f<m_connectivity->size(); ++f)
    {
      boost_foreach (Uint elem, (*m_connectivity)[f])
      {
        if ( elem != Math::Consts::Uint_max() )
        {

          boost::tie(elem_location_comp,elem_location_idx) = lookup().location(elem);

          CCells& elems = elem_location_comp->as_type<CCells>();
          CList<bool>& is_bdry_elem = elems.get_child("is_bdry").as_type< CList<bool> >();
          is_bdry_elem[elem_location_idx] = is_bdry_elem[elem_location_idx] || is_bdry_face.get_row(f) ;
        }
      }
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

std::vector<Uint> CFaceCellConnectivity::face_nodes(const Uint face) const
{
  cf_assert(face < m_connectivity->size());
  cf_assert(face < m_face_nb_in_elem->size());
  Uint unified_elem_idx = (*m_connectivity)[face][0];
  Component::ConstPtr elem_comp;
  Uint elem_idx;

  boost::tie(elem_comp,elem_idx) = lookup().location(unified_elem_idx);

  const CElements& elems = elem_comp->as_type<CElements>();
  std::vector<Uint> nodes(elems.element_type().face_type((*m_face_nb_in_elem)[face][0]).nb_nodes());
  Uint i(0);
  boost_foreach (Uint node_in_face, elems.element_type().face_connectivity().face_node_range((*m_face_nb_in_elem)[face][0]))
  {
    cf_assert(elem_idx < elems.node_connectivity().size());
    nodes[i++] = elems.node_connectivity()[elem_idx][node_in_face];
  }
  return nodes;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
