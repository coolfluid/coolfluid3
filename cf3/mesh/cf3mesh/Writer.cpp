// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/assign/list_of.hpp>

#include "common/BinaryDataWriter.hpp"
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

#include "mesh/cf3mesh/Writer.hpp"
#include "mesh/GeoShape.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"
#include "mesh/Tags.hpp"
#include "mesh/FaceCellConnectivity.hpp"

#include "math/VariablesDescriptor.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace cf3mesh {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < cf3mesh::Writer, MeshWriter, LibCF3Mesh> aCF3MeshWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

namespace detail
{
  // Recursively write all region nodes starting at the given parent component
  void write_regions(common::XML::XmlNode& node, const common::Component& parent, common::BinaryDataWriter& writer, const std::string& root_path)
  {
    BOOST_FOREACH(const Region& region, common::find_components<Region>(parent))
    {
      common::XML::XmlNode region_node = node.add_node("region");
      region_node.set_attribute("name", region.name());
      write_regions(region_node, region, writer, root_path);
      BOOST_FOREACH(const Entities& elements, common::find_components<Entities>(region))
      {
        common::XML::XmlNode elements_node = region_node.add_node("elements");
        elements_node.set_attribute("idx", common::to_str(elements.entities_idx()));
        elements_node.set_attribute("name", elements.name());
        elements_node.set_attribute("element_type", elements.element_type().derived_type_name());
        elements_node.set_attribute("global_indices", common::to_str(writer.append_data(elements.glb_idx())));
        elements_node.set_attribute("ranks", common::to_str(writer.append_data(elements.rank())));

        if( is_not_null(elements.connectivity_cell2face()) )
        {
          using namespace common;
          common::XML::XmlNode connectivity_cell2face = elements_node.add_node("connectivity_cell2face");
          const Uint rows = elements.connectivity_cell2face()->size();
          const Uint cols = elements.connectivity_cell2face()->row_size();
          boost::shared_ptr< Table<Uint> > conn = allocate_component< Table<Uint> >("tmp");
          conn->set_row_size( 2*cols );
          conn->resize( rows );
          for (Uint e=0; e<rows; ++e)
          {
            for (Uint n=0; n<cols; ++n)
            {
              conn->array()[e][n+cols*0] = elements.connectivity_cell2face()->array()[e][n].comp_idx();
              conn->array()[e][n+cols*1] = elements.connectivity_cell2face()->array()[e][n].idx;
            }
          }
          connectivity_cell2face.set_attribute("connectivity", to_str(writer.append_data(*conn)));
        }
        if( is_not_null(elements.connectivity_face2cell()) )
        {
          using namespace common;
          common::XML::XmlNode connectivity_face2cell = elements_node.add_node("connectivity_face2cell");
          const Uint rows = elements.connectivity_face2cell()->size();
          const Uint cols = elements.connectivity_face2cell()->connectivity().row_size();
          boost::shared_ptr< Table<Uint> > conn = allocate_component< Table<Uint> >("tmp");
          conn->set_row_size( 2*cols );
          conn->resize( rows );
          for (Uint e=0; e<rows; ++e)
          {
            for (Uint n=0; n<cols; ++n)
            {
              conn->array()[e][n+cols*0] = elements.connectivity_face2cell()->connectivity()[e][n].comp_idx();
              conn->array()[e][n+cols*1] = elements.connectivity_face2cell()->connectivity()[e][n].idx;
            }
          }
          connectivity_face2cell.set_attribute("connectivity",     to_str(writer.append_data(*conn)));
          connectivity_face2cell.set_attribute("is_bdry_face",     to_str(writer.append_data(elements.connectivity_face2cell()->is_bdry_face())));
          connectivity_face2cell.set_attribute("face_number",      to_str(writer.append_data(elements.connectivity_face2cell()->face_number())));
          connectivity_face2cell.set_attribute("cell_rotation",    to_str(writer.append_data(elements.connectivity_face2cell()->cell_rotation())));
          connectivity_face2cell.set_attribute("cell_orientation", to_str(writer.append_data(elements.connectivity_face2cell()->cell_orientation())));
        }
        if( is_not_null(elements.connectivity_cell2cell()) )
        {
          using namespace common;
          common::XML::XmlNode connectivity_cell2cell = elements_node.add_node("connectivity_cell2cell");
          const Uint rows = elements.connectivity_cell2cell()->size();
          const Uint cols = elements.connectivity_cell2cell()->row_size();
          boost::shared_ptr< Table<Uint> > conn = allocate_component< Table<Uint> >("tmp");
          conn->set_row_size( 2*cols );
          conn->resize( rows );
          for (Uint e=0; e<rows; ++e)
          {
            for (Uint n=0; n<cols; ++n)
            {
              conn->array()[e][n+cols*0] = elements.connectivity_cell2cell()->array()[e][n].comp_idx();
              conn->array()[e][n+cols*1] = elements.connectivity_cell2cell()->array()[e][n].idx;
            }
          }
          connectivity_cell2cell.set_attribute("connectivity", to_str(writer.append_data(*conn)));
        }

        Handle< common::List<Uint> const > periodic_links_elements(elements.get_child("periodic_links_elements"));
        if(is_not_null(periodic_links_elements))
        {
          common::XML::XmlNode periodic_node = elements_node.add_node("periodic_links_elements");
          periodic_node.set_attribute("index", common::to_str(writer.append_data(*periodic_links_elements)));
          Handle< common::Link const > periodic_link(periodic_links_elements->get_child("periodic_link"));
          cf3_always_assert(is_not_null(periodic_link));
          cf3_always_assert(is_not_null(periodic_link->follow()));
          periodic_node.set_attribute("periodic_link", boost::replace_first_copy(periodic_link->follow()->uri().path(), root_path, ""));
        }
      }
    }
  }
  
  // Write a field node into the given XmlNode
  void write_field(common::XML::XmlNode node, const mesh::Field& field, common::BinaryDataWriter& writer)
  {
    common::XML::XmlNode field_node = node.add_node("field");
    field_node.set_attribute("name", field.name());
    field_node.set_attribute("description", field.descriptor().description());
    field_node.set_attribute("table_idx", common::to_str(writer.append_data(field)));
    BOOST_FOREACH(const std::string& tag, field.get_tags())
    {
      field_node.add_node("tag").set_attribute("name", tag);
    }
  }
} // namespace detail

Writer::Writer( const std::string& name )
: MeshWriter(name)
{

}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Writer::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".cf3mesh");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void Writer::write()
{
  common::PE::Comm& comm = common::PE::Comm::instance();
  const Mesh& mesh = *m_mesh;
  const std::string mesh_path = mesh.uri().path() + "/";
  
  // Writer for the arrays
  boost::shared_ptr<common::BinaryDataWriter> data_writer = common::allocate_component<common::BinaryDataWriter>("DataWriter");
  const common::URI binfile = m_file_path.base_path() / (m_file_path.base_name() + ".cfbinxml");
  data_writer->options().set("file", binfile);
  
  common::XML::XmlDoc xml_doc("1.0", "ISO-8859-1");
  common::XML::XmlNode mesh_node = xml_doc.add_node("mesh");
  mesh_node.set_attribute("version", "1");
  mesh_node.set_attribute("binary_file", binfile.path());
  mesh_node.set_attribute("nb_procs", common::to_str(comm.size()));
  
  // Write the dictionaries
  common::XML::XmlNode dictionaries_node = mesh_node.add_node("dictionaries");
  BOOST_FOREACH(const Dictionary& dictionary, common::find_components<Dictionary>(mesh))
  {
    if(dictionary.spaces().empty())
    {
      CFwarn << "Dictionary " << dictionary.name() << " with no spaces will not be written" << CFendl;
      continue;
    }
    
    common::XML::XmlNode dict_node = dictionaries_node.add_node("dictionary");
    dict_node.set_attribute("name", dictionary.name());
    dict_node.set_attribute("continuous", common::to_str(dictionary.continuous()));
    std::string space_lib_name;
    BOOST_FOREACH(const Field& field, common::find_components<Field>(dictionary))
    {
      detail::write_field(dict_node, field, *data_writer);
    }
    dict_node.set_attribute("global_indices", common::to_str(data_writer->append_data(dictionary.glb_idx())));
    dict_node.set_attribute("ranks", common::to_str(data_writer->append_data(dictionary.rank())));
    BOOST_FOREACH(const Handle< Entities >& entities, dictionary.entities_range())
    {
      common::XML::XmlNode entities_node = dict_node.add_node("entities");
      std::string entities_path = entities->uri().path();
      boost::erase_first(entities_path, mesh_path);
      entities_node.set_attribute("path", entities_path);
      entities_node.set_attribute("table_idx", common::to_str(data_writer->append_data(entities->space(dictionary).connectivity())));
      
      if(space_lib_name.empty())
      {
        std::string sf_name = entities->space(dictionary).shape_function().derived_type_name();
        std::vector<std::string> name_parts;
        boost::split(name_parts, sf_name, boost::is_any_of("."));
        space_lib_name = name_parts.front();
        for(Uint i = 1; i != name_parts.size()-1; ++i)
          space_lib_name += "." + name_parts[i];
      }
    }
    
    dict_node.set_attribute("space_lib_name", space_lib_name);
    
    // Add periodic data if needed
    if(dictionary.has_tag(mesh::Tags::geometry()))
    {
      Handle< common::List<Uint> const > periodic_links_nodes(dictionary.get_child("periodic_links_nodes"));
      Handle< common::List<bool> const > periodic_links_active(dictionary.get_child("periodic_links_active"));
      if(is_not_null(periodic_links_nodes) && is_not_null(periodic_links_active))
      {
        dict_node.set_attribute("periodic_links_nodes", common::to_str(data_writer->append_data(*periodic_links_nodes)));
        dict_node.set_attribute("periodic_links_active", common::to_str(data_writer->append_data(*periodic_links_active)));
      }
    }
  }
  
  // Topology and geometry connectivity
  common::XML::XmlNode topology_node = mesh_node.add_node("topology");
  detail::write_regions(topology_node, mesh.topology(), *data_writer, mesh.uri().path() + "/");
  
  if(comm.rank() == 0)
  {
    common::XML::to_file(xml_doc, m_file_path);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // cf3mesh
} // mesh
} // cf3
