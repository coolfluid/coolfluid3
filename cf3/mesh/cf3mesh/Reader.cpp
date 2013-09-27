// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/assign/list_of.hpp>

#include "common/BinaryDataReader.hpp"
#include "common/BoostFilesystem.hpp"
#include "common/Foreach.hpp"
#include "common/OptionList.hpp"
#include "common/Log.hpp"
#include "common/PE/Comm.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/StringConversion.hpp"

#include "common/XML/XmlDoc.hpp"
#include "common/XML/FileOperations.hpp"

#include "mesh/cf3mesh/Reader.hpp"
#include "mesh/GeoShape.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

#include "math/VariablesDescriptor.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace cf3mesh {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < cf3mesh::Reader, MeshReader, LibCF3Mesh> aCF3MeshReader_Builder;

Reader::Reader(const std::string& name): MeshReader(name)
{
  /// TODO: There are shapefunctions defined in this library that are otherwise not found from the buildername :-(
  ///       We should find a solution for this to autoload automatically
  common::Core::instance().libraries().autoload_library_with_namespace("cf3.dcm.core");
}

std::vector< std::string > Reader::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".cf3mesh");
  return extensions;
}

void Reader::do_read_mesh_into(const common::URI& path, Mesh& mesh)
{
  boost::shared_ptr<common::XML::XmlDoc> doc = common::XML::parse_file(path);
  common::XML::XmlNode mesh_node(doc->content->first_node("mesh"));

  common::PE::Comm& comm = common::PE::Comm::instance();
  
  if(!mesh_node.is_valid())
    throw common::FileFormatError(FromHere(), "File " + path.path() + " has no mesh node");
  
  if(mesh_node.attribute_value("version") != "1")
    throw common::FileFormatError(FromHere(), "File " + path.path() + " has incorrect version " + mesh_node.attribute_value("version") + "(expected 1)");

  if(common::from_str<Uint>(mesh_node.attribute_value("nb_procs")) != comm.size())
    throw common::FileFormatError(FromHere(), "File " + path.path() + " has was created for " + mesh_node.attribute_value("nb_procs") + " processes and can't load on " + common::to_str(comm.size()) + " processors");
  
  data_reader = common::allocate_component<common::BinaryDataReader>("DataReader");
  data_reader->options().set("file", common::URI(mesh_node.attribute_value("binary_file")));
  
  common::XML::XmlNode topology_node = mesh_node.content->first_node("topology");
  if(!topology_node.is_valid())
    throw common::FileFormatError(FromHere(), "File " + path.path() + " does has no topology node");
  
  periodic_links_map.clear();
  read_region(topology_node,mesh.topology(),mesh.geometry_fields());
//  common::XML::XmlNode region_node(topology_node.content->first_node("region"));


  // Link the periodic elements, now all elements have been created
  for(PeriodicMapT::const_iterator it = periodic_links_map.begin(); it != periodic_links_map.end(); ++it)
  {
    Handle<common::Link> link = it->first->create_component<common::Link>("periodic_link");
    Handle<Elements> elements_to_link(mesh.access_component(common::URI(it->second, common::URI::Scheme::CPATH)));
    if(is_null(elements_to_link))
      throw common::FileFormatError(FromHere(), "Invalid periodic link: " + it->second);

    link->link_to(*elements_to_link);
  }
  
  common::XML::XmlNode dictionaries_node(mesh_node.content->first_node("dictionaries"));
  if(!dictionaries_node.is_valid())
    throw common::FileFormatError(FromHere(), "File " + path.path() + " does has no dictionaries node");
  
  common::XML::XmlNode dictionary_node(dictionaries_node.content->first_node("dictionary"));
  for(; dictionary_node.is_valid(); dictionary_node.content = dictionary_node.content->next_sibling("dictionary"))
  {
    const std::string dict_name = dictionary_node.attribute_value("name");
    // Add the coordinates first
    if(dict_name == "geometry")
    {
      common::XML::XmlNode field_node(dictionary_node.content->first_node("field"));
      for(; field_node.is_valid(); field_node.content = field_node.content->next_sibling("field"))
      {
        if(field_node.attribute_value("name") == "coordinates")
        {
          const Uint table_idx = common::from_str<Uint>(field_node.attribute_value("table_idx"));
          mesh.initialize_nodes(data_reader->block_rows(table_idx), data_reader->block_cols(table_idx));
          data_reader->read_table(mesh.geometry_fields().coordinates(), table_idx);
        }
      }
      
      // Periodic links
      if(is_not_null(dictionary_node.content->first_attribute("periodic_links_nodes")) && is_not_null(dictionary_node.content->first_attribute("periodic_links_active")))
      {
        cf3_assert(is_null(mesh.geometry_fields().get_child("periodic_links_nodes")));
        cf3_assert(is_null(mesh.geometry_fields().get_child("periodic_links_active")));
        Handle< common::List<Uint> >  periodic_links_nodes = mesh.geometry_fields().create_component< common::List<Uint> >("periodic_links_nodes");
        Handle< common::List<bool> > periodic_links_active = mesh.geometry_fields().create_component< common::List<bool> >("periodic_links_active");
        data_reader->read_list(*periodic_links_nodes, common::from_str<Uint>(dictionary_node.attribute_value("periodic_links_nodes")));
        data_reader->read_list(*periodic_links_active, common::from_str<Uint>(dictionary_node.attribute_value("periodic_links_active")));
      }
    }
  }
  
  dictionary_node.content = dictionaries_node.content->first_node("dictionary");
  for(; dictionary_node.is_valid(); dictionary_node.content = dictionary_node.content->next_sibling("dictionary"))
  {
    const std::string dict_name = dictionary_node.attribute_value("name");
    const std::string space_lib_name = dictionary_node.attribute_value("space_lib_name");
    const bool continuous = common::from_str<bool>(dictionary_node.attribute_value("continuous"));
    
    // The entities used by this dictionary
    std::vector< Handle<Entities> > entities_list;
    std::vector<Uint> entities_binary_file_indices;
    common::XML::XmlNode entities_node = dictionary_node.content->first_node("entities");
    for(; entities_node.is_valid(); entities_node.content = entities_node.content->next_sibling("entities"))
    {
      Handle<Entities> entities(mesh.access_component(common::URI(entities_node.attribute_value("path"), common::URI::Scheme::CPATH)));
      if(is_null(entities))
      {
        CFinfo << mesh.tree() << CFendl;
        throw common::FileFormatError(FromHere(), "Referred entities " + entities_node.attribute_value("path") + " doesn't exist in mesh");
      }
      entities_list.push_back(entities);
      entities_binary_file_indices.push_back(common::from_str<Uint>(entities_node.attribute_value("table_idx")));
    }
    
    Dictionary& dictionary = dict_name == "geometry" ? mesh.geometry_fields() :
      (continuous ? mesh.create_continuous_space(dict_name, space_lib_name, entities_list) : mesh.create_discontinuous_space(dict_name, space_lib_name, entities_list));
      
    // Read the global indices
    data_reader->read_list(dictionary.glb_idx(), common::from_str<Uint>(dictionary_node.attribute_value("global_indices")));
    data_reader->read_list(dictionary.rank(), common::from_str<Uint>(dictionary_node.attribute_value("ranks")));
    
    // Read the fields
    common::XML::XmlNode field_node(dictionary_node.content->first_node("field"));
    for(; field_node.is_valid(); field_node.content = field_node.content->next_sibling("field"))
    {
      if(field_node.attribute_value("name") == "coordinates")
        continue;
      const Uint table_idx = common::from_str<Uint>(field_node.attribute_value("table_idx"));
      Field& field = dictionary.create_field(field_node.attribute_value("name"), field_node.attribute_value("description"));
      common::XML::XmlNode tag_node = field_node.content->first_node("tag");
      for(; tag_node.is_valid(); tag_node.content = tag_node.content->next_sibling("tag"))
        field.add_tag(tag_node.attribute_value("name"));
      
      data_reader->read_table(field, table_idx);
    }
    
    // Read in the connectivity tables
    for(Uint i = 0; i != entities_list.size(); ++i)
    {
      data_reader->read_table(entities_list[i]->space(dictionary).connectivity(), entities_binary_file_indices[i]);
    }
  }

  mesh.update_structures();
  mesh.update_statistics();
  mesh.check_sanity();
  mesh.raise_mesh_loaded();
}

void Reader::read_region(const common::XML::XmlNode& topology_node, Region& topology, Dictionary& geometry)
{
  common::XML::XmlNode region_node(topology_node.content->first_node("region"));
  for(; region_node.is_valid(); region_node.content = region_node.content->next_sibling("region"))
  {
    Region& region = topology.create_region(region_node.attribute_value("name"));

    read_region(region_node,region,geometry);

    common::XML::XmlNode elements_node(region_node.content->first_node("elements"));
    for(; elements_node.is_valid(); elements_node.content = elements_node.content->next_sibling("elements"))
    {
      Elements& elems = region.create_elements(elements_node.attribute_value("element_type"), geometry);
      elems.rename(elements_node.attribute_value("name"));
      data_reader->read_list(elems.glb_idx(), common::from_str<Uint>(elements_node.attribute_value("global_indices")));
      data_reader->read_list(elems.rank(), common::from_str<Uint>(elements_node.attribute_value("ranks")));
      common::XML::XmlNode periodic_node(elements_node.content->first_node("periodic_links_elements"));
      if(periodic_node.is_valid())
      {
        Handle< common::List<Uint> > periodic_links_elements = elems.create_component< common::List<Uint> >("periodic_links_elements");
        data_reader->read_list(*periodic_links_elements, common::from_str<Uint>(periodic_node.attribute_value("index")));
        periodic_links_map.insert(std::make_pair(periodic_links_elements, periodic_node.attribute_value("periodic_link")));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // cf3mesh
} // mesh
} // cf3
