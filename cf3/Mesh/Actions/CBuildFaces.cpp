// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>
#include <boost/foreach.hpp>
#include "common/Log.hpp"
#include "common/CBuilder.hpp"
#include "common/PE/debug.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StreamHelpers.hpp"
#include "common/OptionT.hpp"
#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMeshElements.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CNodeFaceCellConnectivity.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CMesh.hpp"
#include "Math/Functions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
namespace Actions {

  using namespace common;
  using namespace Math::Functions;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CBuildFaces, CMeshTransformer, LibActions> CBuildFaces_Builder;

//////////////////////////////////////////////////////////////////////////////

CBuildFaces::CBuildFaces( const std::string& name )
: CMeshTransformer(name),
  m_store_cell2face(false)
{

  m_properties["brief"] = std::string("Print information of the mesh");
  std::string desc;
  desc =
  "  Usage: Info \n\n"
  "          Information given: internal mesh hierarchy,\n"
  "      element distribution for each region, and element type";
  m_properties["description"] = desc;

  m_options.add_option( OptionT<bool>::create("store_cell2face", m_store_cell2face) )
      ->description("Optionally store Cell to Face connectivity")
      ->pretty_name("Store Cell to Face")
      ->mark_basic()
      ->link_to(&m_store_cell2face);
}

/////////////////////////////////////////////////////////////////////////////

std::string CBuildFaces::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CBuildFaces::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CBuildFaces::execute()
{
  // traverse regions and make interface region between connected regions recursively
  //make_interfaces(m_mesh);
  CMesh& mesh = *m_mesh.lock();
  build_face_cell_connectivity_bottom_up(mesh);
  build_faces_bottom_up(mesh);

  // Add the new faces to the registry of mesh elements
  mesh.elements().update();


  // Now build the cell to face connectivity using the new face indices
  if (m_store_cell2face)
    build_cell_face_connectivity(mesh);
}

//////////////////////////////////////////////////////////////////////////////

void CBuildFaces::make_interfaces(Component& parent)
{
  cf3_assert_desc("parent must be a CRegion or CMesh",
    is_not_null( parent.as_ptr<CMesh>() ) || is_not_null( parent.as_ptr<CRegion>() ) );

  CElements::Ptr comp;
  //std::cout << PERank << "make interfaces for " << parent.name() << std::endl;

  std::vector<CRegion::Ptr> regions = range_to_vector(find_components<CRegion>(parent));
  const Uint n=regions.size();
  if (n>1)
  {
    //Uint nb_interfaces = factorial(n) / (2*factorial(n-2));
    for (Uint i=0; i<n; ++i)
    for (Uint j=i+1; j<n; ++j)
    {
      //std::cout << PERank << "checking interface for " << regions[i]->name() << " to " << regions[j]->name() << std::endl;

      if ( find_components_with_filter<CElements>(*regions[i],IsElementsSurface()).size() != 0 )
      {
        if ( find_components_with_filter<CElements>(*regions[j],IsElementsVolume()).size() !=0 )
        {
          //std::cout << PERank << "matching boundary face to cell for " << regions[i]->name() << " to " << regions[j]->name() << std::endl;

          match_boundary(*regions[i],*regions[j]);
        }
      }
      else if ( find_components_with_filter<CElements>(*regions[j],IsElementsSurface()).size() != 0 )
      {
        if ( find_components_with_filter<CElements>(*regions[i],IsElementsVolume()).size() !=0 )
        {
          //std::cout << PERank << "matching boundary face to cell for " << regions[j]->name() << " to " << regions[i]->name() << std::endl;

          match_boundary(*regions[j],*regions[i]);
        }
      }
      else
      {
        CRegion& interface = parent.create_component<CRegion>("interface_"+regions[i]->name()+"_to_"+regions[j]->name());
        interface.add_tag( Mesh::Tags::interface() );

        //std::cout << PERank << "creating face to cell for interfaces for " << regions[i]->name() << " to " << regions[j]->name() << std::endl;
        CFaceCellConnectivity::Ptr f2c = match_faces(*regions[i],*regions[j]);

        build_face_elements(interface,*f2c,true);

      }

    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CBuildFaces::build_face_cell_connectivity_bottom_up(Component& parent)
{
  cf3_assert_desc("parent must be a CRegion or CMesh",
    is_not_null( parent.as_ptr<CMesh>() ) || is_not_null( parent.as_ptr<CRegion>() ) );

  CCells::Ptr comp;

  boost_foreach( CRegion& region, find_components<CRegion>(parent) )
  {
    build_face_cell_connectivity_bottom_up(region);

    if ( count( find_components_with_filter<CElements>(region,IsElementsVolume()) ) != 0 )
    {
      //std::cout << PERank << "building face_cell connectivity for region " << region.uri().path() << std::endl;
      CFaceCellConnectivity::Ptr face_to_cell = region.create_component_ptr<CFaceCellConnectivity>("face_to_cell");
      face_to_cell->configure_option("face_building_algorithm",true);
      face_to_cell->add_tag(Mesh::Tags::inner_faces());
      face_to_cell->setup(region);
    }
    else
    {
      make_interfaces(region);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CBuildFaces::build_faces_bottom_up(Component& parent)
{
  cf3_assert_desc("parent must be a CRegion or CMesh",
    is_not_null( parent.as_ptr<CMesh>() ) || is_not_null( parent.as_ptr<CRegion>() ) );

  CElements::Ptr comp;

  boost_foreach( CRegion& region, find_components<CRegion>(parent) )
  {
    build_faces_bottom_up(region);

    if ( find_components_with_filter<CElements>(region,IsElementsVolume()).size() != 0 )
    {
      // this region is the bottom region with volume elements

      CFaceCellConnectivity& face_to_cell = find_component<CFaceCellConnectivity>(region);

      //std::cout << PERank << "create cells" << std::endl;
      CRegion& cells = region.create_component<CRegion>("cells");
      boost_foreach(CElements& elements, find_components_with_filter<CElements>(region,IsElementsVolume()))
      {
        elements.move_to(cells);

        if (m_store_cell2face)
        {
          CConnectivity& c2f = elements.create_component<CConnectivity>("face_connectivity");
          c2f.set_lookup(m_mesh.lock()->elements());
          c2f.resize(elements.size());
          c2f.set_row_size(elements.element_type().nb_faces());
        }
      }

      //std::cout << PERank << "create inner faces" << std::endl;
      CRegion& inner_faces = region.create_region(Mesh::Tags::inner_faces());
      build_face_elements(inner_faces,face_to_cell, true);

      //std::cout << PERank << "create outer faces" << std::endl;
      CRegion& outer_faces = region.create_region(Mesh::Tags::outer_faces());
      build_face_elements(outer_faces,face_to_cell, false);
      if (outer_faces.recursive_elements_count() == 0)
        region.remove_component(outer_faces.name());

      region.remove_component(face_to_cell);
    }
    else
    {
      // must use different way, checking from 1 region to another for a match, not just in bdry elements
      // this region is connected to another region
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CBuildFaces::build_face_elements(CRegion& region, CFaceCellConnectivity& face_to_cell, bool is_inner)
{
  CMesh& mesh = *m_mesh.lock();
  std::set<std::string> face_types;
  std::map<std::string,CTable<Uint>::Buffer::Ptr > f2c_buffer_map;
  std::map<std::string,CTable<Uint>::Buffer::Ptr > fnb_buffer_map;
  std::map<std::string,CList<bool>::Buffer::Ptr  > bdry_buffer_map;

  Component::Ptr elem_comp;
  Uint elem_idx;

  CTable<Uint>& face_number = *face_to_cell.get_child_ptr("face_number")->as_ptr< CTable<Uint> >();

  for (Uint f=0; f<face_to_cell.size(); ++f)
  {
    boost::tie(elem_comp,elem_idx) = face_to_cell.lookup().location(face_to_cell.connectivity()[f][0]);
    const Uint face_nb = face_number[f][0];
    face_types.insert(elem_comp->as_type<CElements>().element_type().face_type(face_nb).derived_type_name());
  }

  boost_foreach( const std::string& face_type , face_types)
  {
    const std::string shape_name = build_component_abstract_type<ElementType>(face_type,"tmp")->shape_name();
    CCellFaces& faces = *region.create_component_ptr<CCellFaces>(shape_name);
    //std::cout << PERank << "  creating " << faces.uri().path() << std::endl;
    faces.initialize(face_type,mesh.geometry());
    if (is_inner)
      faces.add_tag(Mesh::Tags::inner_faces());
    else
      faces.add_tag(Mesh::Tags::outer_faces());

    CFaceCellConnectivity& f2c = faces.cell_connectivity();
    CTable<Uint>& raw_table = *f2c.get_child_ptr(f2c.connectivity().name())->as_ptr< CTable<Uint> >();
    raw_table.set_row_size(is_inner?2:1);
    boost_foreach(Component::Ptr cells, face_to_cell.used())
      f2c.add_used(*cells);
    f2c_buffer_map[face_type] = raw_table.create_buffer_ptr();
    fnb_buffer_map[face_type] = f2c.get_child_ptr("face_number")->as_ptr< CTable<Uint> >()->create_buffer_ptr();
    bdry_buffer_map[face_type] = f2c.get_child_ptr("is_bdry_face")->as_ptr< CList<bool> >()->create_buffer_ptr();
  }

  for (Uint f=0; f<face_to_cell.size(); ++f)
  {
    boost::tie(elem_comp,elem_idx) = face_to_cell.lookup().location(face_to_cell.connectivity()[f][0]);
    CElements& elements = elem_comp->as_type<CElements>();
    const Uint face_nb = face_number[f][0];
    const std::string face_type = elements.element_type().face_type(face_nb).derived_type_name();

    if (is_inner)
    {
      if (face_to_cell.is_bdry_face()[f] == false)
      {
        f2c_buffer_map[face_type]->add_row(face_to_cell.connectivity()[f]);
        fnb_buffer_map[face_type]->add_row(face_number[f]);
        bdry_buffer_map[face_type]->add_row(face_to_cell.is_bdry_face()[f]);
      }
    }
    else
    {
      if (face_to_cell.is_bdry_face()[f] == true)
      {
        std::vector<Uint> dummy(1,face_to_cell.connectivity()[f][0]);
        f2c_buffer_map[face_type]->add_row(dummy);
        fnb_buffer_map[face_type]->add_row(face_number[f]);
        bdry_buffer_map[face_type]->add_row(face_to_cell.is_bdry_face()[f]);
      }
    }
  }


  // Check sizes match
  boost_foreach( const std::string& face_type , face_types)
  {
    f2c_buffer_map[face_type]->flush();
    fnb_buffer_map[face_type]->flush();
    bdry_buffer_map[face_type]->flush();

    const std::string shape_name = build_component_abstract_type<ElementType>(face_type,"tmp")->shape_name();
    CCellFaces& faces = *region.get_child_ptr(shape_name)->as_ptr<CCellFaces>();

    faces.rank().resize(faces.size());
    faces.glb_idx().resize(faces.size());
    CFaceCellConnectivity&  f2c  = faces.cell_connectivity();
    CTable<Uint>&           fnb  = f2c.get_child("face_number" ).as_type< CTable<Uint> >();
    CList<bool>&            bdry = f2c.get_child("is_bdry_face").as_type< CList<bool> >();
    cf3_assert(f2c.size() == fnb.size());
    cf3_assert(fnb.size() == faces.size());
    cf3_assert(bdry.size() == faces.size());
  }
}

////////////////////////////////////////////////////////////////////////////////

CFaceCellConnectivity::Ptr CBuildFaces::match_faces(CRegion& region1, CRegion& region2)
{
  CMesh& mesh = *m_mesh.lock();
  // interface connectivity
  CFaceCellConnectivity::Ptr interface = allocate_component<CFaceCellConnectivity>("interface_connectivity");
  interface->configure_option("face_building_algorithm",true);
  CTable<Uint>::Buffer i2c = find_component_with_name<CTable<Uint> >(*interface,Mesh::Tags::connectivity_table()).create_buffer();
  CTable<Uint>::Buffer fnb = find_component_with_name<CTable<Uint> >(*interface,"face_number").create_buffer();
  CList<bool>::Buffer bdry = find_component_with_name<CList<bool> >(*interface,"is_bdry_face").create_buffer();

  // unified face-cell-connectivity from left and right
  CUnifiedData::Ptr Ufaces1 = allocate_component<CUnifiedData>("unified_faces1");
  boost_foreach(CFaceCellConnectivity& inner_f2c, find_components_recursively_with_tag<CFaceCellConnectivity>(region1,Mesh::Tags::inner_faces()))
    Ufaces1->add(inner_f2c);
  CUnifiedData::Ptr Ufaces2 = allocate_component<CUnifiedData>("unified_faces2");
  boost_foreach(CFaceCellConnectivity& inner_f2c, find_components_recursively_with_tag<CFaceCellConnectivity>(region2,Mesh::Tags::inner_faces()))
    Ufaces2->add(inner_f2c);

  // create buffers for each sub-face_cell_connectivity
  std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > buf_fnb1;
  std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > buf_fnb2;
  std::vector<boost::shared_ptr<CList<bool>::Buffer> >  buf_bdryf1;
  std::vector<boost::shared_ptr<CList<bool>::Buffer> >  buf_bdryf2;
  std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > buf_f2c1;
  std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > buf_f2c2;
  std::vector<Uint> elems_1_start_idx;
  std::vector<Uint> elems_2_start_idx;
  Uint nb_elems(0);
  boost_foreach(boost::weak_ptr<Component> faces1_comp, Ufaces1->components())
  {
    CFaceCellConnectivity& faces1 = faces1_comp.lock()->as_type<CFaceCellConnectivity>();
    elems_1_start_idx.push_back(nb_elems);
    buf_fnb1.push_back( boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(faces1.face_number().create_buffer())));
    buf_bdryf1.push_back( boost::shared_ptr<CList<bool>::Buffer> ( new CList<bool>::Buffer(faces1.is_bdry_face().create_buffer())));
    buf_f2c1.push_back( boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(faces1.connectivity().create_buffer())));
    boost_foreach(Component::Ptr cells, faces1.used())
      interface->add_used(cells->as_type<CCells>());
    nb_elems += faces1.lookup().size();
  }

  boost_foreach(boost::weak_ptr<Component> faces2_comp, Ufaces2->components())
  {
    CFaceCellConnectivity& faces2 = faces2_comp.lock()->as_type<CFaceCellConnectivity>();
    elems_2_start_idx.push_back(nb_elems);
    buf_fnb2.push_back( boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(faces2.face_number().create_buffer())));
    buf_bdryf2.push_back( boost::shared_ptr<CList<bool>::Buffer> ( new CList<bool>::Buffer(faces2.is_bdry_face().create_buffer())));
    buf_f2c2.push_back( boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(faces2.connectivity().create_buffer())));
    boost_foreach(Component::Ptr cells, faces2.used())
      interface->add_used(cells->as_type<CCells>());
    nb_elems += faces2.lookup().size();
  }

  // Build a node to face connectivity matching faces2
  CNodeFaceCellConnectivity::Ptr node2faces2_ptr = allocate_component<CNodeFaceCellConnectivity>("node2faces");
  CNodeFaceCellConnectivity& node2faces2 = *node2faces2_ptr;
  boost_foreach(boost::weak_ptr<Component> f2c, Ufaces2->components())
    node2faces2.face_cell_connectivity().add(f2c.lock()->as_type<CFaceCellConnectivity>()); // it is assumed this is only face types
  node2faces2.set_nodes(mesh.geometry());
  node2faces2.build_connectivity();

  Uint f1(0);
  Uint faces1_idx(0);
  boost_foreach(boost::weak_ptr<Component> faces1_comp, Ufaces1->components())
  {
    CFaceCellConnectivity& faces1 = faces1_comp.lock()->as_type<CFaceCellConnectivity>();

    std::vector<Uint> face_nodes;
    std::vector<Uint> elems(2);
    enum {LEFT=0,RIGHT=1};
    Uint nb_matches(0);


    for (Uint f1_idx = 0; f1_idx != faces1.size(); ++f1_idx)
    {
      face_nodes = faces1.face_nodes(f1);
      const Uint nb_nodes_per_face = face_nodes.size();

      std::map<Uint,Uint> found_faces;
      std::map<Uint,Uint>::iterator not_found = found_faces.end();

      bool match_found = false;
      boost_foreach(const Uint face_node, face_nodes)
      {
        boost_foreach(const Uint f2, node2faces2.connectivity()[face_node])
        {
          std::map<Uint,Uint>::iterator it = found_faces.find(f2);
          if ( it == not_found)
          {
            found_faces[f2]=1;
          }
          else
          {
            Uint& nb_faces = it->second;
            ++nb_faces;
            if (nb_faces == nb_nodes_per_face)
            {
              match_found = true;
              Uint f2_idx;
              Uint faces2_idx;
              boost::tie(faces2_idx,f2_idx) = node2faces2.face_cell_connectivity().location_idx(f2);
              CFaceCellConnectivity& faces2 = Ufaces2->components()[faces2_idx].lock()->as_type<CFaceCellConnectivity>();
              elems[LEFT]  = faces1.connectivity()[f1_idx][0] + elems_1_start_idx[faces1_idx];
              elems[RIGHT] = faces2.connectivity()[f2_idx][0] + elems_2_start_idx[faces2_idx];

              //std::cout << PERank << "match found: " << elems[LEFT] << " <--> " << elems[RIGHT] << std::endl;

              // Remove matches from the 2 connectivity tables and add to the interface
              i2c.add_row(elems);
              fnb.add_row(buf_fnb1[faces1_idx]->get_row(f1_idx));
              bdry.add_row(false);

              buf_f2c1[faces1_idx]->rm_row(f1_idx);
              buf_fnb1[faces1_idx]->rm_row(f1_idx);
              buf_bdryf1[faces1_idx]->rm_row(f1_idx);
              buf_f2c2[faces2_idx]->rm_row(f2_idx);
              buf_fnb2[faces2_idx]->rm_row(f2_idx);
              buf_bdryf2[faces2_idx]->rm_row(f2_idx);
              //

              ++nb_matches;
              break;
            }
          }
        }
        if (match_found)
          break;
      }
      ++f1;
    }
    ++faces1_idx;
  }

  return interface;
}

//////////////////////////////////////////////////////////////////////////////

void CBuildFaces::match_boundary(CRegion& bdry_region, CRegion& inner_region)
{
  CMesh& mesh = *m_mesh.lock();
  // unified face-cell-connectivity
  CUnifiedData::Ptr unified_inner_faces_to_cells = allocate_component<CUnifiedData>("unified_inner_faces");
  boost_foreach(CFaceCellConnectivity& f2c, find_components_recursively_with_tag<CFaceCellConnectivity>(inner_region,Mesh::Tags::inner_faces()))
    unified_inner_faces_to_cells->add(f2c);

  // create buffers for each face_cell_connectivity of unified_inner_faces_to_cells
  std::vector<boost::shared_ptr<CTable<Uint>::Buffer> >  buf_vec_inner_face_nb;
  std::vector<boost::shared_ptr<CList<bool>::Buffer> >  buf_vec_inner_face_is_bdry;
  std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > buf_vec_inner_face_connectivity;

  boost_foreach(boost::weak_ptr<Component> inner_faces_comp, unified_inner_faces_to_cells->components())
  {
    CFaceCellConnectivity& inner_faces = inner_faces_comp.lock()->as_type<CFaceCellConnectivity>();
    buf_vec_inner_face_nb.push_back( boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(inner_faces.face_number().create_buffer())));
    buf_vec_inner_face_is_bdry.push_back( boost::shared_ptr<CList<bool>::Buffer> ( new CList<bool>::Buffer(inner_faces.is_bdry_face().create_buffer())));
    buf_vec_inner_face_connectivity.push_back( boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(inner_faces.connectivity().create_buffer())));
  }

  // Build a node to face connectivity matching faces2
  CNodeFaceCellConnectivity::Ptr nodes_to_inner_faces_ptr = allocate_component<CNodeFaceCellConnectivity>("node2faces");
  CNodeFaceCellConnectivity& nodes_to_inner_faces = *nodes_to_inner_faces_ptr;
  boost_foreach(boost::weak_ptr<Component> inner_f2c, unified_inner_faces_to_cells->components())
    nodes_to_inner_faces.face_cell_connectivity().add(inner_f2c.lock()->as_type<CFaceCellConnectivity>());
  nodes_to_inner_faces.set_nodes(mesh.geometry());
  nodes_to_inner_faces.build_connectivity();

  Uint unified_bdry_face_idx(0);
  boost_foreach(CElements& bdry_faces, find_components<CElements>(bdry_region))
  {
    CFaceCellConnectivity::Ptr bdry_face_to_cell = find_component_ptr<CFaceCellConnectivity>(bdry_faces);
    if (is_null(bdry_face_to_cell))
    {
      bdry_face_to_cell = bdry_faces.create_component_ptr<CFaceCellConnectivity>("cell_connectivity");
      bdry_face_to_cell->configure_option("face_building_algorithm",true);
    }

    CTable<Uint>& bdry_face_connectivity = bdry_face_to_cell->connectivity();
    CTable<Uint>& bdry_face_nb = bdry_face_to_cell->face_number();
    CList<bool>& bdry_face_is_bdry = bdry_face_to_cell->is_bdry_face();

    bdry_face_connectivity.set_row_size(1);
    bdry_face_connectivity.resize(bdry_faces.size());
    bdry_face_nb.resize(bdry_faces.size());
    bdry_face_is_bdry.resize(bdry_faces.size());

    std::vector<Uint> vec_inner_cells_start_idx;

    boost_foreach(boost::weak_ptr<Component> inner_cells_comp, unified_inner_faces_to_cells->components())
    {
      CFaceCellConnectivity& inner_face_to_cells = inner_cells_comp.lock()->as_type<CFaceCellConnectivity>();
      boost_foreach(Component::Ptr cells, inner_face_to_cells.used())
        bdry_face_to_cell->add_used(cells->as_type<CCells>());
    }

    // initialize a row of size 1, in which connected cells will be stored, and copied into
    // the bdry_face_connectivity table
    std::vector<Uint> elems(1);

    // initialize a counter for see if matches are found.
    // A match is found if every node of a boundary face is also found in an inner_face
    Uint nb_matches(0);

    Uint local_bdry_face_idx(0);
    boost_foreach(CConnectivity::ConstRow bdry_face_nodes, bdry_faces.node_connectivity().array())
    {
      const Uint nb_nodes_per_face = bdry_face_nodes.size();



      std::map<Uint,Uint> found_faces;
      std::map<Uint,Uint>::iterator not_found = found_faces.end();

      bool match_found = false;
      boost_foreach(const Uint bdry_face_node, bdry_face_nodes)
      {
        boost_foreach(const Uint unified_inner_face_idx, nodes_to_inner_faces.connectivity()[bdry_face_node])
        {
          std::map<Uint,Uint>::iterator it = found_faces.find(unified_inner_face_idx);
          if ( it == not_found)
          {
            found_faces[unified_inner_face_idx]=1;
            if (nb_nodes_per_face == 1)
            {
              match_found = true;

              CFaceCellConnectivity::Ptr inner_faces_to_cells;
              Uint inner_faces_comp_idx;
              Uint inner_face_idx;
              boost::tie(inner_faces_comp_idx,inner_face_idx) = nodes_to_inner_faces.face_cell_connectivity().location_idx(unified_inner_face_idx);
              inner_faces_to_cells = unified_inner_faces_to_cells->components()[inner_faces_comp_idx].lock()->as_ptr<CFaceCellConnectivity>();
              elems[0] = inner_faces_to_cells->connectivity()[inner_face_idx][0];

              //std::cout << PERank << "match found: " << unified_bdry_face_idx << " <--> " << elems[0] << std::endl;

              // Remove matches from the inner_faces_connectivity tables and add to the boundary
              bdry_face_connectivity.set_row(local_bdry_face_idx,elems);
              bdry_face_nb[local_bdry_face_idx] = buf_vec_inner_face_nb[inner_faces_comp_idx]->get_row(inner_face_idx);
              bdry_face_is_bdry[local_bdry_face_idx] = true;

              buf_vec_inner_face_connectivity[inner_faces_comp_idx]->rm_row(inner_face_idx);
              buf_vec_inner_face_nb[inner_faces_comp_idx]->rm_row(inner_face_idx);
              buf_vec_inner_face_is_bdry[inner_faces_comp_idx]->rm_row(inner_face_idx);

              ++nb_matches;
              break;
            }
          }
          else
          {
            Uint& nb_faces = it->second;
            ++nb_faces;
            if (nb_faces == nb_nodes_per_face)
            {
              match_found = true;
              CFaceCellConnectivity::Ptr inner_faces_to_cells;
              Uint inner_face_idx;
              Uint inner_faces_comp_idx;
              boost::tie(inner_faces_comp_idx,inner_face_idx) = nodes_to_inner_faces.face_cell_connectivity().location_idx(unified_inner_face_idx);
              inner_faces_to_cells = unified_inner_faces_to_cells->components()[inner_faces_comp_idx].lock()->as_ptr<CFaceCellConnectivity>();
              elems[0] = inner_faces_to_cells->connectivity()[inner_face_idx][0];

              // ----------- debug -----------
              //Component::Ptr inner_cell_comp;
              //Uint inner_cell_idx;
              //boost::tie(inner_cell_comp,inner_cell_idx) = inner_faces_to_cells->lookup().location(inner_faces_to_cells->connectivity()[inner_face_idx][0]);
              //std::cout << PERank << "match found: " << bdry_faces.parent().name() << "/" << bdry_faces.name() << "["<<local_bdry_face_idx<<"] <--> " << inner_cell_comp->parent().name()<<"/"<<inner_cell_comp->name()<<"["<<inner_cell_idx<<"]" << std::endl;
              // -----------------------------

               // RealMatrix cell_coordinates = inner_cell_comp->as_type<CElements>().get_coordinates(inner_cell_idx);
               // RealVector face_coordinates = bdry_faces.get_coordinates(local_bdry_face_idx).row(0);
               // bool match_found_double_check = false;
               // for (Uint i=0; i<cell_coordinates.rows(); ++i)
               // {
               //   if (cell_coordinates.row(i) == face_coordinates.transpose())
               //   {
               //     match_found_double_check = true;
               //     break;
               //   }
               // }
               // cf3_assert(match_found_double_check);

              // Remove matches from the 2 connectivity tables and add to the interface
              bdry_face_connectivity.set_row(local_bdry_face_idx,elems);
              bdry_face_nb[local_bdry_face_idx] = buf_vec_inner_face_nb[inner_faces_comp_idx]->get_row(inner_face_idx);
              bdry_face_is_bdry[local_bdry_face_idx] = true;

              buf_vec_inner_face_connectivity[inner_faces_comp_idx]->rm_row(inner_face_idx);
              buf_vec_inner_face_nb[inner_faces_comp_idx]->rm_row(inner_face_idx);
              buf_vec_inner_face_is_bdry[inner_faces_comp_idx]->rm_row(inner_face_idx);

              ++nb_matches;
              break;
            }
          }
        }
        if (match_found)
          break;
      }
      ++unified_bdry_face_idx;
      ++local_bdry_face_idx;
    }
  }

  for (Uint vec_idx=0; vec_idx<buf_vec_inner_face_connectivity.size(); ++vec_idx)
  {
    buf_vec_inner_face_connectivity[vec_idx]->flush();
    buf_vec_inner_face_nb[vec_idx]->flush();
    buf_vec_inner_face_is_bdry[vec_idx]->flush();
  }

}

//////////////////////////////////////////////////////////////////////////////

void CBuildFaces::build_cell_face_connectivity(Component& parent)
{
  Component::Ptr cells;
  Uint cell_idx;
  Uint unified_cell_idx;
  Uint face_nb_idx;
  boost_foreach(CEntities& face_elements, find_components_recursively_with_tag<CEntities>(parent,Mesh::Tags::face_entity()) )
  {
    //std::cout << PERank << face_elements.uri().path() << std::endl;
    CFaceCellConnectivity& f2c = face_elements.get_child("cell_connectivity").as_type<CFaceCellConnectivity>();
    const CTable<Uint>& connectivity = f2c.connectivity();
    const CList<bool>& is_bdry       = f2c.is_bdry_face();
    const CTable<Uint>& face_nb      = f2c.face_number();

    for (Uint face_idx=0; face_idx<face_elements.size(); ++face_idx)
    {
      //std::cout << PERank << "    face["<<face_idx<<"]" << std::endl;;
      unified_cell_idx = connectivity[face_idx][LEFT];
      face_nb_idx      = face_nb[face_idx][LEFT];
      boost::tie(cells,cell_idx)=f2c.lookup().location(unified_cell_idx);
      //std::cout << PERank << "        --->  cell["<<cell_idx<<"]"<< std::endl;
      CConnectivity& c2f = cells->get_child("face_connectivity").as_type<CConnectivity>();
      c2f[cell_idx][face_nb_idx] = c2f.lookup().unified_idx(face_elements,face_idx);
      if (is_bdry[face_idx] == false)
      {
        unified_cell_idx = connectivity[face_idx][RIGHT];
        face_nb_idx      = face_nb[face_idx][RIGHT];
        boost::tie(cells,cell_idx)=f2c.lookup().location(unified_cell_idx);
        c2f = cells->get_child("face_connectivity").as_type<CConnectivity>();
        cf3_assert(cell_idx < c2f.size());
        cf3_assert(face_nb_idx < c2f.row_size());
        c2f[cell_idx][face_nb_idx] = c2f.lookup().unified_idx(face_elements,face_idx);
        //std::cout << PERank << "        --->  cell[" << cell_idx<<"]"<< std::endl;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // cf3
