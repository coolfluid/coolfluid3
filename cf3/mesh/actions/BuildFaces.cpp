// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StreamHelpers.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "common/PE/debug.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/Buffer.hpp"

#include "math/Functions.hpp"
#include "math/Consts.hpp"

#include "mesh/CellFaces.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/NodeElementConnectivity.hpp"
#include "mesh/Node2FaceCellConnectivity.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

#include "mesh/actions/BuildFaces.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace math::Functions;

struct FaceCompare
{
  bool operator()(const Face2Cell& face1, const Face2Cell& face2) const
  {
    std::size_t seed1=0;
    std::size_t seed2=0;

    boost::hash_combine(seed1,face1.comp->uri().string());
    boost::hash_combine(seed2,face2.comp->uri().string());
    boost::hash_combine(seed1,face1.idx);
    boost::hash_combine(seed2,face2.idx);
    return seed1 < seed2;
  }
};

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BuildFaces, MeshTransformer, mesh::actions::LibActions> BuildFaces_Builder;

//////////////////////////////////////////////////////////////////////////////

BuildFaces::BuildFaces( const std::string& name )
: MeshTransformer(name),
  m_store_cell2face(false)
{

  properties()["brief"] = std::string("Print information of the mesh");
  std::string desc;
  desc =
  "  Usage: Info \n\n"
  "          Information given: internal mesh hierarchy,\n"
  "      element distribution for each region, and element type";
  properties()["description"] = desc;

  options().add_option("store_cell2face", m_store_cell2face)
      .description("Optionally store Cell to Face connectivity")
      .pretty_name("Store Cell to Face")
      .mark_basic()
      .link_to(&m_store_cell2face);
}

/////////////////////////////////////////////////////////////////////////////

std::string BuildFaces::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string BuildFaces::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void BuildFaces::execute()
{
  // traverse regions and make interface region between connected regions recursively
  //make_interfaces(m_mesh);
  Mesh& mesh = *m_mesh;
  PE::Comm::instance().barrier();
  build_face_cell_connectivity_bottom_up(mesh);

  build_faces_bottom_up(mesh);

  // Now build the cell to face connectivity using the new face indices
  if (m_store_cell2face)
    build_cell_face_connectivity(mesh);

  mesh.update_statistics();
  mesh.update_structures();
}

//////////////////////////////////////////////////////////////////////////////

void BuildFaces::make_interfaces(Component& parent)
{
  cf3_assert_desc("parent must be a Region or Mesh",
    is_not_null( Handle<Mesh>(parent.handle<Component>()) ) || is_not_null( Handle<Region>(parent.handle<Component>()) ) );

  Handle< Elements > comp;
  //PECheckArrivePoint(100,"make interfaces for " << parent.name())
  std::vector<Handle< Region > > regions = range_to_vector(find_components<Region>(parent));
  const Uint n=regions.size();
  if (n>1)
  {
    //Uint nb_interfaces = factorial(n) / (2*factorial(n-2));
    for (Uint i=0; i<n; ++i)
    for (Uint j=i+1; j<n; ++j)
    {
      CFdebug << PERank << "checking interface for " << regions[i]->name() << " to " << regions[j]->name() << CFendl;

      if ( find_components_with_filter<Elements>(*regions[i],IsElementsSurface()).size() != 0 )
      {
        if ( find_components_with_filter<Elements>(*regions[j],IsElementsVolume()).size() !=0 )
        {
          CFdebug << PERank << "  --> matching boundary face to cell for " << regions[i]->name() << " to " << regions[j]->name() << CFendl;

          match_boundary(*regions[i],*regions[j]);
        }
      }
      else if ( find_components_with_filter<Elements>(*regions[j],IsElementsSurface()).size() != 0 )
      {
        if ( find_components_with_filter<Elements>(*regions[i],IsElementsVolume()).size() !=0 )
        {
          CFdebug << PERank << "  --> matching boundary face to cell for " << regions[j]->name() << " to " << regions[i]->name() << CFendl;

          match_boundary(*regions[j],*regions[i]);
        }
      }
      else
      {
        Region& interface = *parent.create_component<Region>("interface_"+regions[i]->name()+"_to_"+regions[j]->name());
        interface.add_tag( mesh::Tags::interface() );

        CFdebug << PERank << "creating face to cell for interfaces for " << regions[i]->name() << " to " << regions[j]->name() << CFendl;
        boost::shared_ptr< FaceCellConnectivity > f2c = match_faces(*regions[i],*regions[j]);
        //PECheckArrivePoint(100,"finished matching faces for "<< regions[i]->name() << " to " << regions[j]->name());
        CFdebug << PERank << "creating face elements inside " << regions[i]->name() << " to " << regions[j]->name() << CFendl;
        build_face_elements(interface,*f2c,true);
        boost_foreach(Entities& entities, find_components_recursively<Entities>(interface))
            entities.add_tag( mesh::Tags::interface() );
        //PECheckArrivePoint(100,"finished creating face elements inside "<< regions[i]->name() << " to " << regions[j]->name());
      }

    }
  }
  //PECheckPoint(100,"finished making interfaces for " << parent.name())
}

////////////////////////////////////////////////////////////////////////////////

void BuildFaces::build_face_cell_connectivity_bottom_up(Component& parent)
{
  cf3_assert_desc("parent must be a Region or Mesh",
    is_not_null( Handle<Mesh>(parent.handle<Component>()) ) || is_not_null( Handle<Region>(parent.handle<Component>()) ) );

  Handle< Cells > comp;

  boost_foreach( Region& region, find_components<Region>(parent) )
  {
    build_face_cell_connectivity_bottom_up(region);
    if ( count( find_components_with_filter<Elements>(region,IsElementsVolume()) ) != 0 )
    {
//      CFdebug << PERank << "building face_cell connectivity for region " << region.uri().path() << CFendl;
      Handle<FaceCellConnectivity> face_to_cell = region.create_component<FaceCellConnectivity>("face_to_cell");
      face_to_cell->options().configure_option("face_building_algorithm",true);
      face_to_cell->add_tag(mesh::Tags::inner_faces());
      face_to_cell->setup(region);
      PE::Comm::instance().barrier();
    }
    else
    {
      make_interfaces(region);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void BuildFaces::build_faces_bottom_up(Component& parent)
{
  //PECheckArrivePoint(100,"building faces inside region " << parent.uri())
  cf3_assert_desc("parent must be a Region or Mesh",
    is_not_null( Handle<Mesh>(parent.handle<Component>()) ) || is_not_null( Handle<Region>(parent.handle<Component>()) ) );

  Handle< Elements > comp;

  boost_foreach( Region& region, find_components<Region>(parent) )
  {
    build_faces_bottom_up(region);

    if ( find_components_with_filter<Elements>(region,IsElementsVolume()).size() != 0 )
    {
      // this region is the bottom region with volume elements

      FaceCellConnectivity& face_to_cell = find_component<FaceCellConnectivity>(region);

//      CFdebug << PERank << "create cells" << CFendl;
      Region& cells = *region.create_component<Region>("cells");
      cells.add_tag(mesh::Tags::cells());
      boost_foreach(Elements& elements, find_components_with_filter<Elements>(region,IsElementsVolume()))
      {
        elements.move_to(cells);
      }

//      CFdebug << PERank << "create inner faces" << CFendl;
      Region& inner_faces = region.create_region(mesh::Tags::inner_faces());
      inner_faces.add_tag(mesh::Tags::inner_faces());
      build_face_elements(inner_faces,face_to_cell, true);

//      CFdebug << PERank << "create outer faces" << CFendl;
      Region& outer_faces = region.create_region(mesh::Tags::outer_faces());
      outer_faces.add_tag(mesh::Tags::outer_faces());
      build_face_elements(outer_faces,face_to_cell, false);
      if (outer_faces.global_elements_count(true) == 0)
        region.remove_component(outer_faces.name());

      region.remove_component(face_to_cell);
    }
    else
    {
      // must use different way, checking from 1 region to another for a match, not just in bdry elements
      // this region is connected to another region
    }
  }
  //PECheckPoint(100,"finished building faces inside region " << parent.uri())
}

////////////////////////////////////////////////////////////////////////////////

void BuildFaces::build_face_elements(Region& region, FaceCellConnectivity& face_to_cell, bool is_inner)
{
  Mesh& mesh = *m_mesh;
  std::set<std::string> face_types;
  std::map<std::string,boost::shared_ptr< ElementConnectivity::Buffer > > f2c_buffer_map;
  std::map<std::string,boost::shared_ptr< common::Table<Uint>::Buffer > > fnb_buffer_map;
  std::map<std::string,boost::shared_ptr< common::List<bool>::Buffer > > bdry_buffer_map;

  Handle< Component > elem_comp;
  Uint elem_idx;

  common::Table<Uint>& face_number = *Handle< common::Table<Uint> >(face_to_cell.get_child("face_number"));

  for (Uint idx=0; idx<face_to_cell.size(); ++idx)
  {
    Face2Cell face(face_to_cell,idx);
    face_types.insert( face.element_type().derived_type_name()  );
  }

  if (PE::Comm::instance().is_active())
  {
    PE::Buffer face_types_send;
    PE::Buffer face_types_recv;

    boost_foreach(const std::string& ftype, face_types)
      face_types_send << ftype;

    //PECheckPoint(100,"check_start");
    face_types_send.all_gather(face_types_recv);
    //PECheckPoint(100,"check_stop");
    face_types.clear();
    std::string dummy;
    while (face_types_recv.more_to_unpack())
    {
      face_types_recv >> dummy;
      face_types.insert(dummy);
    }

    // for debug
    Uint loc_nb_face_types = face_types.size();
    Uint glb_nb_face_types(0);
    PE::Comm::instance().all_reduce(PE::max(),&loc_nb_face_types,1,&glb_nb_face_types);
    int pass = (loc_nb_face_types==glb_nb_face_types);
    PE::Comm::instance().all_reduce(PE::logical_and(),&pass,1,&pass);
    cf3_assert_desc("tree will not be synchrone!!!",pass);
    // end for debug
  }
  boost_foreach( const std::string& face_type , face_types)
  {
    const std::string shape_name = build_component_abstract_type<ElementType>(face_type,"tmp")->shape_name();
    CellFaces& faces = *region.create_component<CellFaces>(shape_name);
    //CFdebug << PERank << "  creating " << faces.uri().path() << CFendl;
    faces.initialize(face_type,mesh.geometry_fields());
    if (is_inner)
      faces.add_tag(mesh::Tags::inner_faces());
    else
      faces.add_tag(mesh::Tags::outer_faces());


    faces.connectivity_face2cell() = faces.create_component<FaceCellConnectivity>("cell_connectivity");
    FaceCellConnectivity& f2c = *faces.connectivity_face2cell();

    ElementConnectivity& raw_table = *Handle< ElementConnectivity >(f2c.get_child(f2c.connectivity().name()));
    raw_table.set_row_size(is_inner?2:1);
    boost_foreach(Handle< Component > cells, face_to_cell.used())
      f2c.add_used(*cells);
    f2c_buffer_map[face_type] = raw_table.create_buffer_ptr();
    fnb_buffer_map[face_type] = Handle< common::Table<Uint> >(f2c.get_child("face_number"))->create_buffer_ptr();
    bdry_buffer_map[face_type] = Handle< common::List<bool> >(f2c.get_child("is_bdry_face"))->create_buffer_ptr();
  }

  for (Uint f=0; f<face_to_cell.size(); ++f)
  {
    Entity element = face_to_cell.connectivity()[f][0];
    if ( is_null(element.comp) )
      throw InvalidStructure(FromHere(),"Face matching messed up in region "+region.uri().string());
    const Uint face_nb = face_number[f][0];
    const std::string face_type = element.element_type().face_type(face_nb).derived_type_name();

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
        std::vector<Entity> dummy(1,face_to_cell.connectivity()[f][0]);
        f2c_buffer_map[face_type]->add_row(dummy);
        fnb_buffer_map[face_type]->add_row(face_number[f]);
        bdry_buffer_map[face_type]->add_row(face_to_cell.is_bdry_face()[f]);
      }
    }
  }

  boost_foreach( const std::string& face_type , face_types)
  {
    f2c_buffer_map[face_type]->flush();
    fnb_buffer_map[face_type]->flush();
    bdry_buffer_map[face_type]->flush();

    const std::string shape_name = build_component_abstract_type<ElementType>(face_type,"tmp")->shape_name();
    CellFaces& faces = *Handle<CellFaces>(region.get_child(shape_name));
    FaceCellConnectivity&  f2c  = *faces.connectivity_face2cell();

    // Add shortcut to Entities component
    cf3_assert(f2c.handle<FaceCellConnectivity>());
    faces.connectivity_face2cell() = f2c.handle<FaceCellConnectivity>();
    cf3_assert(faces.connectivity_face2cell());

    faces.geometry_space().connectivity().set_row_size(faces.geometry_space().shape_function().nb_nodes());
    faces.resize(f2c.size());
    for (Uint f=0; f<faces.size(); ++f)
    {
      if (PE::Comm::instance().size()==1)
      {
        faces.rank()[f] = f2c.connectivity()[f][LEFT].rank();
      }
      else
      {
        if (faces.connectivity_face2cell()->is_bdry_face()[f] == false)
        {
          faces.rank()[f] = math::Consts::uint_max();
          /// @todo make restore following, when Dictionary rank finding is done automatically for entire topology
//          if (f2c.connectivity()[f][LEFT].rank()  != PE::Comm::instance().rank() &&
//              f2c.connectivity()[f][RIGHT].rank() != PE::Comm::instance().rank())
//          {
//            faces.rank()[f] = math::Consts::uint_max();
//          }
//          else
//          {
//            faces.rank()[f] = std::min(f2c.connectivity()[f][LEFT].rank(),f2c.connectivity()[f][RIGHT].rank());
//          }
        }
        else
        {
          // It is impossible to know if another cpu of lower rank owns this face as well.
          faces.rank()[f] = math::Consts::uint_max();
        }
      }
      faces.glb_idx()[f]= math::Consts::uint_max();
      faces.geometry_space().connectivity().set_row(f,f2c.face_nodes(f));
    }

    Handle< common::Table<Uint> >          fnb(f2c.get_child("face_number"));
    Handle< common::List<bool> >           bdry(f2c.get_child("is_bdry_face"));
    cf3_assert_desc(to_str(f2c.size())+"=="+to_str(fnb->size()),(f2c.size() == fnb->size()));
    cf3_assert_desc(to_str(fnb->size())+"=="+to_str(faces.size()),fnb->size() == faces.size());
    cf3_assert_desc(to_str(bdry->size())+"=="+to_str(faces.size()),bdry->size() == faces.size());
  }

  if (PE::Comm::instance().is_active())
  {
    Uint loc_nb_childs = find_components(region).size();
    Uint glb_nb_childs = 0;
    PE::Comm::instance().all_reduce(PE::max(),&loc_nb_childs,1,&glb_nb_childs);
    cf3_assert(loc_nb_childs==glb_nb_childs);
  }
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< FaceCellConnectivity > BuildFaces::match_faces(Region& region1, Region& region2)
{

  CFdebug << "matching faces between regions " << region1.uri().path() << "  and  " << region2.uri().path() << CFendl;

  Mesh& mesh = *m_mesh;
  // interface connectivity
  boost::shared_ptr<FaceCellConnectivity> interface = allocate_component<FaceCellConnectivity>("interface_connectivity");
  interface->options().configure_option("face_building_algorithm",true);
  ElementConnectivity::Buffer i2c = find_component_with_name<ElementConnectivity>(*interface,mesh::Tags::connectivity_table()).create_buffer();
  common::Table<Uint>::Buffer fnb = find_component_with_name<common::Table<Uint> >(*interface,"face_number").create_buffer();
  common::List<bool>::Buffer bdry = find_component_with_name<common::List<bool> >(*interface,"is_bdry_face").create_buffer();


  // create buffers for each sub-face_cell_connectivity
  std::map<FaceCellConnectivity*,boost::shared_ptr<common::Table<Uint>::Buffer> > buf_fnb;
  std::map<FaceCellConnectivity*,boost::shared_ptr<common::List<bool>::Buffer> >  buf_bdry;
  std::map<FaceCellConnectivity*,boost::shared_ptr<ElementConnectivity::Buffer> > buf_f2c;

  // Build a node to face connectivity matching faces2
  boost::shared_ptr<Node2FaceCellConnectivity> node2faces2_ptr = allocate_component<Node2FaceCellConnectivity>("node2faces");
  Node2FaceCellConnectivity& node2faces2 = *node2faces2_ptr;
  boost_foreach(FaceCellConnectivity& faces2, find_components_recursively_with_tag<FaceCellConnectivity>(region2,mesh::Tags::inner_faces()))
  {
    buf_fnb [&faces2] = boost::shared_ptr<common::Table<Uint>::Buffer> ( new common::Table<Uint>::Buffer(faces2.face_number().create_buffer()));
    buf_bdry[&faces2] = boost::shared_ptr<common::List<bool>::Buffer> ( new common::List<bool>::Buffer(faces2.is_bdry_face().create_buffer()));
    buf_f2c [&faces2] = boost::shared_ptr<ElementConnectivity::Buffer> ( new ElementConnectivity::Buffer(faces2.connectivity().create_buffer()));
    node2faces2.add_used(faces2); // it is assumed this is only face types
  }
  node2faces2.set_nodes(mesh.geometry_fields());
  node2faces2.build_connectivity();

  Uint f1(0);
  Uint faces1_idx(0);
  boost_foreach(FaceCellConnectivity& faces1, find_components_recursively_with_tag<FaceCellConnectivity>(region1,mesh::Tags::inner_faces()))
  {
    buf_fnb [&faces1] = boost::shared_ptr<common::Table<Uint>::Buffer> ( new common::Table<Uint>::Buffer(faces1.face_number().create_buffer()));
    buf_bdry[&faces1] = boost::shared_ptr<common::List<bool>::Buffer> ( new common::List<bool>::Buffer(faces1.is_bdry_face().create_buffer()));
    buf_f2c [&faces1] = boost::shared_ptr<ElementConnectivity::Buffer> ( new ElementConnectivity::Buffer(faces1.connectivity().create_buffer()));


    std::vector<Uint> face_nodes;
    std::vector<Entity> elems(2);
    std::vector<Uint> face_nb(2);
    enum {LEFT=0,RIGHT=1};
    Uint nb_matches(0);


    for (Uint idx=0; idx<faces1.size(); ++idx)
    {
      Face2Cell face1(faces1,idx);
      face_nodes = face1.nodes();
      const Uint nb_nodes_per_face = face_nodes.size();

      std::map<Face2Cell,Uint,FaceCompare> found_faces;
      std::map<Face2Cell,Uint,FaceCompare>::iterator not_found = found_faces.end();

      bool match_found = false;
      boost_foreach(const Uint face_node, face_nodes)
      {
        boost_foreach(const Face2Cell face2, node2faces2.connectivity()[face_node])
        {
          std::map<Face2Cell,Uint,FaceCompare>::iterator it = found_faces.find(face2);
          if ( it == not_found)
          {
            found_faces[face2]=1;
          }
          else
          {
            Uint& nb_faces = it->second;
            ++nb_faces;
            if (nb_faces == nb_nodes_per_face)
            {
              match_found = true;
              elems[LEFT]  = face1.cells()[0];
              elems[RIGHT] = face2.cells()[0];
              face_nb[LEFT] = face1.face_nb_in_cells()[0];
              face_nb[RIGHT] = face2.face_nb_in_cells()[0];
              CFdebug << PERank << "match found: " << elems[LEFT] << " <--> " << elems[RIGHT] << CFendl;

              // Remove matches from the 2 connectivity tables and add to the interface
              i2c.add_row(elems);
              fnb.add_row(face_nb);
//              fnb.add_row(buf_fnb[face1.comp]->get_row(face1.idx));
              bdry.add_row(false);

              buf_f2c [face1.comp]->rm_row(face1.idx);
              buf_f2c [face2.comp]->rm_row(face2.idx);
              buf_fnb [face1.comp]->rm_row(face1.idx);
              buf_fnb [face2.comp]->rm_row(face2.idx);
              buf_bdry[face1.comp]->rm_row(face1.idx);
              buf_bdry[face2.comp]->rm_row(face2.idx);

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


void BuildFaces::match_boundary(Region& bdry_region, Region& inner_region)
{
  Mesh& mesh = *m_mesh;

  // create buffers for each face_cell_connectivity of unified_inner_faces_to_cells
  std::map<FaceCellConnectivity*,boost::shared_ptr<common::Table<Uint>::Buffer> >  buf_inner_face_nb;
  std::map<FaceCellConnectivity*,boost::shared_ptr<common::List<bool>::Buffer> >   buf_inner_face_is_bdry;
  std::map<FaceCellConnectivity*,boost::shared_ptr<ElementConnectivity::Buffer> >  buf_inner_face_connectivity;

  // Build a node to face connectivity matching faces2
  boost::shared_ptr<Node2FaceCellConnectivity> nodes_to_inner_faces_ptr = allocate_component<Node2FaceCellConnectivity>("node2faces");
  Node2FaceCellConnectivity& nodes_to_inner_faces = *nodes_to_inner_faces_ptr;
  boost_foreach(FaceCellConnectivity& f2c, find_components_recursively_with_tag<FaceCellConnectivity>(inner_region,mesh::Tags::inner_faces()))
  {
    buf_inner_face_nb          [&f2c] = boost::shared_ptr<common::Table<Uint>::Buffer> ( new common::Table<Uint>::Buffer(f2c.face_number().create_buffer()));
    buf_inner_face_is_bdry     [&f2c] = boost::shared_ptr<common::List<bool>::Buffer>  ( new common::List<bool>::Buffer(f2c.is_bdry_face().create_buffer()));
    buf_inner_face_connectivity[&f2c] = boost::shared_ptr<ElementConnectivity::Buffer> ( new ElementConnectivity::Buffer(f2c.connectivity().create_buffer()));
    nodes_to_inner_faces.add_used(f2c);
  }
  nodes_to_inner_faces.set_nodes(mesh.geometry_fields());
  nodes_to_inner_faces.build_connectivity();

  boost_foreach(Elements& bdry_faces, find_components<Elements>(bdry_region))
  {
//    bdry_faces.add_tag(mesh::Tags::outer_faces());
    Handle< FaceCellConnectivity > bdry_face_to_cell = find_component_ptr<FaceCellConnectivity>(bdry_faces);
    if (is_null(bdry_face_to_cell))
    {
      bdry_face_to_cell = bdry_faces.create_component<FaceCellConnectivity>("cell_connectivity");
      bdry_face_to_cell->options().configure_option("face_building_algorithm",true);

      bdry_faces.connectivity_face2cell() = bdry_face_to_cell;
    }

    ElementConnectivity& bdry_face_connectivity = bdry_face_to_cell->connectivity();
    common::Table<Uint>& bdry_face_nb = bdry_face_to_cell->face_number();
    common::List<bool>& bdry_face_is_bdry = bdry_face_to_cell->is_bdry_face();

    bdry_face_connectivity.set_row_size(1);
    bdry_face_connectivity.resize(bdry_faces.size());
    bdry_face_nb.resize(bdry_faces.size());
    bdry_face_is_bdry.resize(bdry_faces.size());

    // initialize a row of size 1, in which connected cells will be stored, and copied into
    // the bdry_face_connectivity table
    std::vector<Entity> elems(1);

    // initialize a counter for see if matches are found.
    // A match is found if every node of a boundary face is also found in an inner_face
    Uint nb_matches(0);
    for (Uint idx=0; idx<bdry_faces.size(); ++idx)
    {
      Entity bdry_entity(bdry_faces,idx);
      Connectivity::ConstRow bdry_face_nodes = bdry_entity.get_nodes();
      const Uint nb_nodes_per_face = bdry_face_nodes.size();

      std::map<Face2Cell,Uint,FaceCompare> found_faces;
      std::map<Face2Cell,Uint,FaceCompare>::iterator not_found = found_faces.end();

      //      CFdebug << "    searching for match for " << bdry_entity << "  ( ";
      //      boost_foreach(const Uint bdry_face_node, bdry_face_nodes)
      //          CFdebug << bdry_face_node << " " ;
      //      CFdebug << ")"<< CFendl;
      bool match_found = false;
      boost_foreach(const Uint bdry_face_node, bdry_face_nodes)
      {
        //        CFdebug << "         node = " << bdry_face_node << CFendl;
        boost_foreach( Face2Cell& inner_face, nodes_to_inner_faces.connectivity()[bdry_face_node])
        {
          //          CFdebug << "           check inner_face["<<inner_face.idx<<"]  ( ";
          //          boost_foreach(const Uint face_node, inner_face.nodes())
          //              CFdebug << face_node << " ";
          //          CFdebug << " )"<<CFendl;
          std::map<Face2Cell,Uint,FaceCompare>::iterator it = found_faces.find(inner_face);
          if ( it == not_found)
          {
            found_faces[inner_face]=1;

            it = found_faces.find(inner_face);
            //            CFdebug << "               bump " << it->first.comp->uri().path() << "  " << it->first.idx << CFendl;

            if (nb_nodes_per_face == 1)
            {
              match_found = true;
              elems[0] = inner_face.cells()[0];

              //              CFdebug << PERank << "match found: " << inner_face.comp->uri().string()<<"["<<inner_face.idx<<"]" << " <--> " << elems[0] << CFendl;

              // Remove matches from the inner_faces_connectivity tables and add to the boundary
              bdry_face_connectivity.set_row(bdry_entity.idx,elems);
              bdry_face_nb[bdry_entity.idx][0] = inner_face.face_nb_in_cells()[0];
              bdry_face_is_bdry[bdry_entity.idx] = true;

              buf_inner_face_connectivity[inner_face.comp]->rm_row(inner_face.idx);
              buf_inner_face_nb[inner_face.comp]->rm_row(inner_face.idx);
              buf_inner_face_is_bdry[inner_face.comp]->rm_row(inner_face.idx);

              ++nb_matches;
              break;
            }
          }
          else
          {
            Uint& nb_faces = it->second;
            ++nb_faces;
            //            CFdebug << "               bump " << it->first.comp->uri().path() << "  " << it->first.idx << CFendl;

            if (nb_faces == nb_nodes_per_face)
            {

              match_found = true;
              elems[0] = inner_face.cells()[0];

              CFdebug << PERank << "match found: " << inner_face.comp->uri().string()<<"["<<inner_face.idx<<"]" << " <--> " << elems[0] << CFendl;

              // Remove matches from the inner_faces_connectivity tables and add to the boundary
              bdry_face_connectivity.set_row(bdry_entity.idx,elems);
              bdry_face_nb[bdry_entity.idx][0] = inner_face.face_nb_in_cells()[0];
              bdry_face_is_bdry[bdry_entity.idx] = true;

              buf_inner_face_connectivity[inner_face.comp]->rm_row(inner_face.idx);
              buf_inner_face_nb[inner_face.comp]->rm_row(inner_face.idx);
              buf_inner_face_is_bdry[inner_face.comp]->rm_row(inner_face.idx);

              ++nb_matches;
              break;




//              match_found = true;
//              Handle< FaceCellConnectivity > inner_faces_to_cells;
//              Uint inner_face_idx;
//              Uint inner_faces_comp_idx;
//              boost::tie(inner_faces_comp_idx,inner_face_idx) = nodes_to_inner_faces.face_cell_connectivity().location_idx(unified_inner_face_idx);
//              inner_faces_to_cells = Handle<FaceCellConnectivity>(unified_inner_faces_to_cells->components()[inner_faces_comp_idx]);
//              elems[0] = inner_faces_to_cells->connectivity()[inner_face_idx][0];

//              // ----------- debug -----------
//              //Handle< Component > inner_cell_comp;
//              //Uint inner_cell_idx;
//              //boost::tie(inner_cell_comp,inner_cell_idx) = inner_faces_to_cells->lookup().location(inner_faces_to_cells->connectivity()[inner_face_idx][0]);
//              //CFdebug << PERank << "match found: " << bdry_faces.parent().name() << "/" << bdry_faces.name() << "["<<local_bdry_face_idx<<"] <--> " << inner_cell_comp->parent().name()<<"/"<<inner_cell_comp->name()<<"["<<inner_cell_idx<<"]" << CFendl;
//              // -----------------------------

//               // RealMatrix cell_coordinates = inner_cell_comp->as_type<Elements>().get_coordinates(inner_cell_idx);
//               // RealVector face_coordinates = bdry_faces.get_coordinates(local_bdry_face_idx).row(0);
//               // bool match_found_double_check = false;
//               // for (Uint i=0; i<cell_coordinates.rows(); ++i)
//               // {
//               //   if (cell_coordinates.row(i) == face_coordinates.transpose())
//               //   {
//               //     match_found_double_check = true;
//               //     break;
//               //   }
//               // }
//               // cf3_assert(match_found_double_check);

//              // Remove matches from the 2 connectivity tables and add to the interface
//              bdry_face_connectivity.set_row(local_bdry_face_idx,elems);
//              bdry_face_nb[local_bdry_face_idx] = buf_vec_inner_face_nb[inner_faces_comp_idx]->get_row(inner_face_idx);
//              bdry_face_is_bdry[local_bdry_face_idx] = true;

//              buf_vec_inner_face_connectivity[inner_faces_comp_idx]->rm_row(inner_face_idx);
//              buf_vec_inner_face_nb[inner_faces_comp_idx]->rm_row(inner_face_idx);
//              buf_vec_inner_face_is_bdry[inner_faces_comp_idx]->rm_row(inner_face_idx);

//              ++nb_matches;
//              break;
            }
          }
        }
        if (match_found)
          break;
      }
    }
  }

}

//////////////////////////////////////////////////////////////////////////////

void BuildFaces::build_cell_face_connectivity(Component& parent)
{

  boost_foreach(Cells& elements, find_components_recursively<Cells>(parent))
  {
    ElementConnectivity& c2f = *elements.create_component<ElementConnectivity>("face_connectivity");
    c2f.resize(elements.size());
    c2f.set_row_size(elements.element_type().nb_faces());

    // Add shortcut to Entities component
    elements.connectivity_cell2face() = c2f.handle<ElementConnectivity>();
  }

  boost_foreach(Entities& face_elements, find_components_recursively_with_tag<Entities>(parent,mesh::Tags::face_entity()) )
  {
//    PECheckPoint(100,"building c2f for " << face_elements.uri());
    CFdebug << PERank << face_elements.uri().path() << CFendl;
    FaceCellConnectivity& f2c = *face_elements.get_child("cell_connectivity")->handle<FaceCellConnectivity>();
    const ElementConnectivity& connectivity = f2c.connectivity();
    const common::List<bool>& is_bdry       = f2c.is_bdry_face();
    const common::Table<Uint>& face_nb      = f2c.face_number();

    for (Uint idx=0; idx<face_elements.size(); ++idx)
    {
      Face2Cell face(f2c,idx);
      Entity left_cell = face.cells()[LEFT];
      CFdebug << PERank << "    face "<< face.comp->uri()<< "["<<face.idx<<"]" << CFendl;
      CFdebug << PERank << "        --->  cell "<< left_cell.comp->uri() << "["<<left_cell.idx<<"]["<<face.face_nb_in_cells()[LEFT]<<"]"<< CFendl;

      ElementConnectivity& left_c2f = *left_cell.comp->get_child("face_connectivity")->handle<ElementConnectivity>();
      cf3_assert(left_cell.idx < left_c2f.size());
      cf3_assert(face.face_nb_in_cells()[LEFT] < left_c2f[left_cell.idx].size());
      left_c2f[left_cell.idx][face.face_nb_in_cells()[LEFT]] = Entity(face_elements,idx);
      cf3_assert(left_c2f[left_cell.idx][face.face_nb_in_cells()[LEFT]].comp);
      if (face.is_bdry() == false)
      {
        Entity right_cell = face.cells()[RIGHT];
        CFdebug << PERank << "        --->  cell" << right_cell.comp->uri() << "[" << right_cell.idx<<"]["<<face.face_nb_in_cells()[RIGHT]<<"]"<< CFendl;

        ElementConnectivity& right_c2f = *right_cell.comp->get_child("face_connectivity")->handle<ElementConnectivity>();
        cf3_assert(right_cell.idx < right_c2f.size());
        cf3_assert(face.face_nb_in_cells()[RIGHT] < right_c2f[right_cell.idx].size());
        right_c2f[right_cell.idx][face.face_nb_in_cells()[RIGHT]] = Entity(face_elements,idx);
        cf3_assert(right_c2f[right_cell.idx][face.face_nb_in_cells()[RIGHT]].comp);
      }
    }
//    PECheckArrivePoint(100,"built " << face_elements.uri());
  }
}

//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
