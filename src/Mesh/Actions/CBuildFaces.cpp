// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CNodeFaceCellConnectivity.hpp"

#include "Math/MathFunctions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
  using namespace Math::MathFunctions;
    
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBuildFaces, CMeshTransformer, LibActions> CBuildFaces_Builder;

//////////////////////////////////////////////////////////////////////////////

CBuildFaces::CBuildFaces( const std::string& name )
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

std::string CBuildFaces::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CBuildFaces::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CBuildFaces::transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args)
{

  m_mesh = mesh;

  // traverse regions and make interface region between connected regions recursively
  //make_interfaces(m_mesh);
  build_face_cell_connectivity_bottom_up(m_mesh);
  build_faces_bottom_up(m_mesh);
}

//////////////////////////////////////////////////////////////////////////////

void CBuildFaces::make_interfaces(Component::Ptr parent)
{
  cf_assert_desc("parent must be a CRegion or CMesh", 
    is_not_null( parent->as_type<CMesh>() ) || is_not_null( parent->as_type<CRegion>() ) );
 
  CElements::Ptr comp;
  Uint idx_in_comp;
  
  boost::shared_ptr<matched_faces_t> matched_faces;
  std::vector<CRegion::Ptr> regions = range_to_vector(find_components<CRegion>(*parent));
  const Uint n=regions.size();
  if (n>1)
  {
    //Uint nb_interfaces = factorial(n) / (2*factorial(n-2));
    for (Uint i=0; i<n; ++i)
    for (Uint j=i+1; j<n; ++j)
    {
      if ( find_components_with_filter<CElements>(*regions[i],IsElementsSurface()).size() != 0 )
      {
        if ( find_components_with_filter<CElements>(*regions[j],IsElementsVolume()).size() !=0 )
        {
          CFinfo << "matching boundary face to cell for " << regions[i]->name() << " to " << regions[j]->name() << CFendl;
          
          match_boundary(*regions[i],*regions[j]);
        }
      }
      else if ( find_components_with_filter<CElements>(*regions[j],IsElementsSurface()).size() != 0 )
      {
        if ( find_components_with_filter<CElements>(*regions[i],IsElementsVolume()).size() !=0 )
        {
          CFinfo << "matching boundary face to cell for " << regions[j]->name() << " to " << regions[i]->name() << CFendl;
          
          match_boundary(*regions[j],*regions[i]);
        }
      }
      else
      {
        CRegion& interface = *parent->create_component<CRegion>("interface_"+regions[i]->name()+"_to_"+regions[j]->name());
        interface.add_tag("interface");

        CFinfo << "creating face to cell for interfaces for " << regions[i]->name() << " to " << regions[j]->name() << CFendl;
        CFaceCellConnectivity::Ptr f2c = match_faces(*regions[i],*regions[j]);

        build_face_elements(interface,*f2c,true);
        
      }

    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CBuildFaces::build_face_cell_connectivity_bottom_up(Component::Ptr parent)
{
  cf_assert_desc("parent must be a CRegion or CMesh", 
    is_not_null( parent->as_type<CMesh>() ) || is_not_null( parent->as_type<CRegion>() ) );
  
  CElements::Ptr comp;
  Uint idx_in_comp;
  
  boost_foreach( CRegion& region, find_components<CRegion>(*parent) )
  {
    build_face_cell_connectivity_bottom_up(region.self());
    
    if ( count( find_components_with_filter<CElements>(region,IsElementsVolume()) ) != 0 )
    {
      // this region is the bottom region with volume elements

      // create region for cells
      //CRegion& cells = region.create_region("cells");
      //boost_foreach(CElements& elements, find_components_with_filter<CElements>(region,IsElementsVolume()))
      //{
      //  elements.move_to(cells.self());
      //}

      CFaceCellConnectivity::Ptr face_to_cell = region.create_component<CFaceCellConnectivity>("face_to_cell");
      face_to_cell->add_tag("inner_faces");
      CFinfo << "creating face to cell for inner cells of " << region.full_path().path() << CFendl;
      face_to_cell->setup(region);

      for (Uint i=0; i<face_to_cell->size(); ++i)
      {
        CFinfo << "face ["<<i<<"] :  ";
        Uint j=0;
        boost_foreach( const Uint elem_idx, face_to_cell->connectivity()[i] )
        {
          if (j==1 && face_to_cell->is_bdry_face()[i])
            continue;
          boost::tie(comp,idx_in_comp) = face_to_cell->element_location(elem_idx);
          CFinfo << "   " << comp->glb_idx()[idx_in_comp];  
          ++j;
        }
        CFinfo << CFendl;
      }
      
      //CFinfo << "create inner faces" << CFendl;
      //CRegion& inner_faces = region.create_region("inner_faces");
      //build_face_elements(inner_faces,*face_to_cell, true);
      
      // CFinfo << "create outer faces" << CFendl;
      // CRegion& outer_faces = region.create_region("outer_faces");
      // build_face_elements(outer_faces,*face_to_cell, false);
    }
    else
    { 
      // must use different way, checking from 1 region to another for a match, not just in bdry elements
      // this region is connected to another region
      make_interfaces(region.self());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CBuildFaces::build_faces_bottom_up(Component::Ptr parent)
{
  cf_assert_desc("parent must be a CRegion or CMesh", 
    is_not_null( parent->as_type<CMesh>() ) || is_not_null( parent->as_type<CRegion>() ) );
  
  CElements::Ptr comp;
  Uint idx_in_comp;
  
  boost_foreach( CRegion& region, find_components<CRegion>(*parent) )
  {
    build_faces_bottom_up(region.self());
    
    if ( find_components_with_filter<CElements>(region,IsElementsVolume()).size() != 0 )
    {
      // this region is the bottom region with volume elements

      CFaceCellConnectivity::Ptr face_to_cell = find_component_ptr<CFaceCellConnectivity>(region);

      CFinfo << "create cells" << CFendl;
      CRegion::Ptr cells = region.create_component<CRegion>("cells");
      boost_foreach(CElements& elements, find_components_with_filter<CElements>(region,IsElementsVolume()))
        elements.move_to(cells);

      CFinfo << "create inner faces" << CFendl;
      CRegion& inner_faces = region.create_region("inner_faces");
      build_face_elements(inner_faces,*face_to_cell, true);
      
      CFinfo << "create outer faces" << CFendl;
      CRegion& outer_faces = region.create_region("outer_faces");
      build_face_elements(outer_faces,*face_to_cell, false);
      if (outer_faces.recursive_elements_count() == 0)
        region.remove_component(outer_faces.name());
      
      region.remove_component(face_to_cell->name());
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
  std::set<std::string> face_types;
  std::map<std::string,boost::shared_ptr<CTable<Uint>::Buffer> > f2c_buffer_map;
  std::map<std::string,boost::shared_ptr<CTable<Uint>::Buffer> > f2n_buffer_map; 
  std::map<std::string,boost::shared_ptr<CList<Uint>::Buffer>  > fnb_buffer_map; 
  
  CElements::Ptr elem_comp;
  Uint elem_idx;
  
  CList<Uint>& face_number = *face_to_cell.get_child<CList<Uint> >("face_number");
  
  for (Uint f=0; f<face_to_cell.size(); ++f)
  {
    boost::tie(elem_comp,elem_idx) = face_to_cell.element_location(face_to_cell.connectivity()[f][0]);
    const Uint face_nb = face_number[f];
    face_types.insert(elem_comp->element_type().face_type(face_nb).element_type_name());
  }
  
  boost_foreach( const std::string& face_type , face_types)
  {
    CElements& elements = region.create_elements(face_type,m_mesh->nodes());
    if (is_inner)
      elements.add_tag("inner_faces");
    else
      elements.add_tag("outer_faces");
    
    CTable<Uint>& f2n = elements.connectivity_table();
    f2n_buffer_map[face_type] = boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(f2n.create_buffer()) );
    
    CFaceCellConnectivity& f2c = *elements.create_component<CFaceCellConnectivity>("cell_connectivity");
    CTable<Uint>& raw_table = *f2c.get_child<CTable<Uint> >(f2c.connectivity().name());
    raw_table.set_row_size(is_inner?2:1);
    f2c.add_elements(face_to_cell.get_child<CUnifiedData<CElements> >("elements"));
    f2c_buffer_map[face_type] = boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(raw_table.create_buffer()) );
    fnb_buffer_map[face_type] = boost::shared_ptr<CList<Uint>::Buffer>  ( new CList<Uint>::Buffer (f2c.get_child<CList<Uint> > ("face_number")->create_buffer()) );
  }
  
  for (Uint f=0; f<face_to_cell.size(); ++f)
  {
    boost::tie(elem_comp,elem_idx) = face_to_cell.element_location(face_to_cell.connectivity()[f][0]);
    const Uint face_nb = face_number[f];
    const std::string face_type = elem_comp->element_type().face_type(face_nb).element_type_name();

    if (is_inner)
    {
      if (face_to_cell.is_bdry_face()[f] == false)
      {
        f2c_buffer_map[face_type]->add_row(face_to_cell.connectivity()[f]);
        fnb_buffer_map[face_type]->add_row(face_number[f]);
        f2n_buffer_map[face_type]->add_row(face_to_cell.nodes(f));
      }      
    }
    else
    {
      if (face_to_cell.is_bdry_face()[f] == true)
      {
        std::vector<Uint> dummy(1,face_to_cell.connectivity()[f][0]);
        f2c_buffer_map[face_type]->add_row(dummy);
        fnb_buffer_map[face_type]->add_row(face_number[f]);
        f2n_buffer_map[face_type]->add_row(face_to_cell.nodes(f));
      }
    }
  }
  
  
  // Check sizes match
  boost_foreach( const std::string& face_type , face_types)
  {
    f2c_buffer_map[face_type]->flush();
    fnb_buffer_map[face_type]->flush();
    f2n_buffer_map[face_type]->flush();
    
    CElements& elements = region.elements("elements_"+face_type);
    
    CTable<Uint>&           f2n = elements.connectivity_table();
    CFaceCellConnectivity&  f2c = *elements.get_child<CFaceCellConnectivity>("cell_connectivity");
    CList<Uint>&            fnb = *f2c.get_child<CList<Uint> > ("face_number");
    cf_assert(f2n.size() == f2c.size());
    cf_assert(f2c.size() == fnb.size());
  }
  
  
}

////////////////////////////////////////////////////////////////////////////////

CFaceCellConnectivity::Ptr CBuildFaces::match_faces(CRegion& region1, CRegion& region2)
{
  
  // interface connectivity
  CFaceCellConnectivity::Ptr interface = allocate_component<CFaceCellConnectivity>("interface_connectivity");
  CTable<Uint>::Buffer i2c = find_component<CTable<Uint> >(*interface).create_buffer();
  CList<Uint>::Buffer fnb = find_component_with_name<CList<Uint> >(*interface,"face_number").create_buffer();
  CList<Uint>::Buffer bdry = find_component_with_name<CList<Uint> >(*interface,"is_bdry_face").create_buffer();
  
  // unified face-cell-connectivity from left and right
  CUnifiedData<CFaceCellConnectivity>::Ptr Ufaces1 = allocate_component<CUnifiedData<CFaceCellConnectivity> >("unified_faces1");
  Ufaces1->add_data(find_components_recursively_with_tag<CFaceCellConnectivity>(region1,"inner_faces").as_vector());
  CUnifiedData<CFaceCellConnectivity>::Ptr Ufaces2 = allocate_component<CUnifiedData<CFaceCellConnectivity> >("unified_faces2");
  Ufaces2->add_data(find_components_recursively_with_tag<CFaceCellConnectivity>(region2,"inner_faces").as_vector());
  
  // create buffers for each sub-face_cell_connectivity
  std::vector<boost::shared_ptr<CList<Uint>::Buffer> >  buf_fnb1;
  std::vector<boost::shared_ptr<CList<Uint>::Buffer> >  buf_fnb2;
  std::vector<boost::shared_ptr<CList<Uint>::Buffer> >  buf_bdryf1;
  std::vector<boost::shared_ptr<CList<Uint>::Buffer> >  buf_bdryf2;
  std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > buf_f2c1;
  std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > buf_f2c2;
  std::vector<Uint> elems_1_start_idx;
  std::vector<Uint> elems_2_start_idx;
  Uint nb_elems(0);
  boost_foreach(CFaceCellConnectivity::Ptr faces1, Ufaces1->data_components())
  {
    elems_1_start_idx.push_back(nb_elems);
    buf_fnb1.push_back( boost::shared_ptr<CList<Uint>::Buffer> ( new CList<Uint>::Buffer(find_component_with_name<CList<Uint> >(*faces1,"face_number").create_buffer())));
    buf_bdryf1.push_back( boost::shared_ptr<CList<Uint>::Buffer> ( new CList<Uint>::Buffer(find_component_with_name<CList<Uint> >(*faces1,"is_bdry_face").create_buffer())));
    buf_f2c1.push_back( boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(find_component<CTable<Uint> >(*faces1).create_buffer())));
    interface->add_elements(find_component_ptr<CUnifiedData<CElements> >(*faces1));
    nb_elems += find_component<CUnifiedData<CElements> >(*faces1).size();
  }
  
  boost_foreach(CFaceCellConnectivity::Ptr faces2, Ufaces2->data_components())
  {
    elems_2_start_idx.push_back(nb_elems);
    buf_fnb2.push_back( boost::shared_ptr<CList<Uint>::Buffer> ( new CList<Uint>::Buffer(find_component_with_name<CList<Uint> >(*faces2,"face_number").create_buffer())));
    buf_bdryf2.push_back( boost::shared_ptr<CList<Uint>::Buffer> ( new CList<Uint>::Buffer(find_component_with_name<CList<Uint> >(*faces2,"is_bdry_face").create_buffer())));
    buf_f2c2.push_back( boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(find_component<CTable<Uint> >(*faces2).create_buffer())));
    interface->add_elements(find_component_ptr<CUnifiedData<CElements> >(*faces2));
    nb_elems += find_component<CUnifiedData<CElements> >(*faces2).size();    
  }

  // Build a node to face connectivity matching faces2
  CNodeFaceCellConnectivity::Ptr node2faces2_ptr = allocate_component<CNodeFaceCellConnectivity>("node2faces");
  CNodeFaceCellConnectivity& node2faces2 = *node2faces2_ptr;
  node2faces2.add_face_cell_connectivity(Ufaces2->data_components()); // it is assumed this is only face types
  node2faces2.set_nodes(m_mesh->nodes());
  node2faces2.build_connectivity();
  
  
  Uint f1(0);
  index_foreach(faces1_idx, CFaceCellConnectivity::Ptr faces1, Ufaces1->data_components())
  {
    std::vector<Uint> face_nodes;
    std::vector<Uint> elems(2);
    enum {LEFT=0,RIGHT=1};
    Uint nb_matches(0);
    
    
    for (Uint f1_idx = 0; f1_idx != faces1->size(); ++f1_idx)
    {
      face_nodes = faces1->nodes(f1);
      const Uint nb_nodes_per_face = face_nodes.size();
      
      Uint nb_matched_nodes = 0;
  
      std::map<Uint,Uint> found_faces;
      std::map<Uint,Uint>::iterator not_found = found_faces.end();
  
      bool match_found = false;
      boost_foreach(const Uint face_node, face_nodes)
      {
        boost_foreach(const Uint f2, node2faces2.faces(face_node))
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
              CFaceCellConnectivity::Ptr faces2;
              Uint f2_idx;
              Uint faces2_idx;
              boost::tie(faces2_idx,f2_idx) = node2faces2.face_local_idx(f2);
              faces2 = Ufaces2->data_components()[faces2_idx];
              elems[LEFT]  = faces1->elements(f1_idx)[0] + elems_1_start_idx[faces1_idx];
              elems[RIGHT] = faces2->elements(f2_idx)[0] + elems_2_start_idx[faces2_idx];
              
              CFinfo << "match found: " << elems[LEFT] << " <--> " << elems[RIGHT] << CFendl;
              
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
  }
  
  return interface;
}

//////////////////////////////////////////////////////////////////////////////

void CBuildFaces::match_boundary(CRegion& bdry_region, CRegion& region2)
{
  
  // unified face-cell-connectivity
  CUnifiedData<CFaceCellConnectivity>::Ptr Ufaces2 = allocate_component<CUnifiedData<CFaceCellConnectivity> >("unified_faces2");
  Ufaces2->add_data(find_components_recursively_with_tag<CFaceCellConnectivity>(region2,"inner_faces").as_vector());
  
  // create buffers for each sub-face_cell_connectivity
  std::vector<boost::shared_ptr<CList<Uint>::Buffer> >  buf_fnb2;
  std::vector<boost::shared_ptr<CList<Uint>::Buffer> >  buf_bdryf2;
  std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > buf_f2c2;
  
  boost_foreach(CFaceCellConnectivity::Ptr faces2, Ufaces2->data_components())
  {
    buf_fnb2.push_back( boost::shared_ptr<CList<Uint>::Buffer> ( new CList<Uint>::Buffer(find_component_with_name<CList<Uint> >(*faces2,"face_number").create_buffer())));
    buf_bdryf2.push_back( boost::shared_ptr<CList<Uint>::Buffer> ( new CList<Uint>::Buffer(find_component_with_name<CList<Uint> >(*faces2,"is_bdry_face").create_buffer())));
    buf_f2c2.push_back( boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(find_component<CTable<Uint> >(*faces2).create_buffer())));
  }

  // Build a node to face connectivity matching faces2
  CNodeFaceCellConnectivity::Ptr node2faces2_ptr = allocate_component<CNodeFaceCellConnectivity>("node2faces");
  CNodeFaceCellConnectivity& node2faces2 = *node2faces2_ptr;
  node2faces2.add_face_cell_connectivity(Ufaces2->data_components()); // it is assumed this is only face types
  node2faces2.set_nodes(m_mesh->nodes());
  node2faces2.build_connectivity();
  
  
  
  Uint f1(0);
  boost_foreach(CElements& faces1, find_components<CElements>(bdry_region))
  {
    CFaceCellConnectivity::Ptr bdry_connectivity = find_component_ptr<CFaceCellConnectivity>(faces1);
    if (is_null(bdry_connectivity))
      bdry_connectivity = faces1.create_component<CFaceCellConnectivity>("cell_connectivity");

    find_component<CTable<Uint> >(*bdry_connectivity).set_row_size(1);
    CTable<Uint>::Buffer f2c = find_component<CTable<Uint> >(*bdry_connectivity).create_buffer();
    CList<Uint>::Buffer fnb = find_component_with_name<CList<Uint> >(*bdry_connectivity,"face_number").create_buffer();
    CList<Uint>::Buffer bdry = find_component_with_name<CList<Uint> >(*bdry_connectivity,"is_bdry_face").create_buffer();
    
    std::vector<Uint> elems_2_start_idx;
    Uint nb_elems = find_component<CUnifiedData<CElements> >(*bdry_connectivity).size();
    boost_foreach(CFaceCellConnectivity::Ptr faces2, Ufaces2->data_components())
    {
      elems_2_start_idx.push_back(nb_elems);
      nb_elems += find_component<CUnifiedData<CElements> >(*faces2).size();
      bdry_connectivity->add_elements(find_component_ptr<CUnifiedData<CElements> >(*faces2));
      CFLogVar(find_component<CTable<Uint> >(*faces2).size());
    }

    std::vector<Uint> elems(1);
    Uint nb_matches(0);
    
    boost_foreach(CTable<Uint>::ConstRow face_nodes, faces1.connectivity_table().array())
    {
      const Uint nb_nodes_per_face = face_nodes.size();
      
      Uint nb_matched_nodes = 0;
  
      std::map<Uint,Uint> found_faces;
      std::map<Uint,Uint>::iterator not_found = found_faces.end();
  
      bool match_found = false;
      boost_foreach(const Uint face_node, face_nodes)
      {
        boost_foreach(const Uint f2, node2faces2.faces(face_node))
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
              CFaceCellConnectivity::Ptr faces2;
              Uint f2_idx;
              Uint faces2_idx;
              boost::tie(faces2_idx,f2_idx) = node2faces2.face_local_idx(f2);
              faces2 = Ufaces2->data_components()[faces2_idx];
              elems[0] = faces2->elements(f2_idx)[0] + elems_2_start_idx[faces2_idx];
              
              CFinfo << "match found: " << f1 << " <--> " << elems[0] << CFendl;
              
              // Remove matches from the 2 connectivity tables and add to the interface
              f2c.add_row(elems);
              fnb.add_row(buf_fnb2[faces2_idx]->get_row(f2_idx));
              bdry.add_row(true);
              
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
  }
  
  for (Uint faces2_idx=0; faces2_idx<buf_fnb2.size(); ++faces2_idx)
  {
    buf_f2c2[faces2_idx]->flush();
    buf_fnb2[faces2_idx]->flush();
    buf_bdryf2[faces2_idx]->flush();
  }
  
  boost_foreach(CFaceCellConnectivity::Ptr faces2, Ufaces2->data_components())
  {
    CFLogVar(find_component<CTable<Uint> >(*faces2).size());
  }
  
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
