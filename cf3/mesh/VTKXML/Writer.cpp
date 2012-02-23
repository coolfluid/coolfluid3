// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/cstdint.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/BoostFilesystem.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/PE/Comm.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/StringConversion.hpp"

#include "common/XML/FileOperations.hpp"
#include "common/XML/XmlDoc.hpp"
#include "common/XML/XmlNode.hpp"

#include "mesh/VTKXML/Writer.hpp"
#include "mesh/GeoShape.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;

namespace cf3 {
namespace mesh {
namespace VTKXML {

namespace detail
{
  struct CompressedStreamHeader
  {
    CompressedStreamHeader() :
      nb_blocks(1),
      blocksize(32768), // Same as in ParaView
      last_blocksize(0)
    {
    }

    boost::uint32_t nb_blocks;
    const boost::uint32_t blocksize;
    boost::uint32_t last_blocksize;
    std::vector<boost::uint32_t> compressed_blocksizes;
  };

  struct CompressedStream
  {
    CompressedStream() :
      data_stream(std::ios_base::in | std::ios_base::out | std::ios_base::binary),
      m_current_block(std::ios_base::in | std::ios_base::out | std::ios_base::binary)
    {
      // VTK data starts with a _
      data_stream.write("_", 1);
    }

    /// Start writing a new array
    void start_array(const Uint nb_elems, const Uint wordsize)
    {
      m_wordsize = wordsize;

      const Uint nb_bytes = nb_elems * wordsize;
      m_header.last_blocksize = nb_bytes % m_header.blocksize;
      if(m_header.last_blocksize)
      {
        m_header.nb_blocks = nb_bytes / m_header.blocksize + 1;
      }
      else
      {
        m_header.last_blocksize = m_header.blocksize;
        m_header.nb_blocks = nb_bytes / m_header.blocksize;
      }

      m_header.compressed_blocksizes.clear();
      m_header.compressed_blocksizes.reserve(m_header.nb_blocks);

      // Write known header info
      data_stream.write(reinterpret_cast<const char*>(&m_header.nb_blocks), 4);
      data_stream.write(reinterpret_cast<const char*>(&m_header.blocksize), 4);
      data_stream.write(reinterpret_cast<const char*>(&m_header.last_blocksize), 4);

      // save filepointer
      m_compressed_sizes_start = data_stream.tellp();

      // Reserve space for compressed block sizes
      for(Uint i = 0; i != m_header.nb_blocks; ++i)
        data_stream.write(reinterpret_cast<const char*>(&m_header.nb_blocks), 4);
    }

    /// Finish writing the current array
    void finish_array()
    {
      // Write the last block
      cf3_assert(static_cast<Uint>(m_current_block.tellp()) == static_cast<Uint>(m_header.last_blocksize));
      compress_block();

      // go back to the header
      const Uint stream_end = data_stream.tellp();
      data_stream.seekp(m_compressed_sizes_start);

      // Write actual compressed block sizes
      for(Uint i = 0; i != m_header.nb_blocks; ++i)
        data_stream.write(reinterpret_cast<const char*>(&m_header.compressed_blocksizes[i]), 4);

      // go back to the stream end
      data_stream.seekp(stream_end);
    }

    /// Append a value to the stream
    template<typename ValueT>
    void push_back(const ValueT& value)
    {
      // Block is full, write it to the compressed stream
      if(m_current_block.tellp() == static_cast<std::streampos>(m_header.blocksize))
        compress_block();

      m_current_block.write(reinterpret_cast<const char*>(&value), m_wordsize);
    }

    // Offset to put in the VTK XML (= offset after the _)
    Uint offset()
    {
      return static_cast<Uint>(data_stream.tellp()) - 1u;
    }

    // Compress the current block and append it to the data stream
    void compress_block()
    {
      // set up zlib compressor
      boost::iostreams::filtering_stream<boost::iostreams::input> compressed_stream;
      compressed_stream.push(boost::iostreams::zlib_compressor());
      compressed_stream.push(m_current_block);

      // write data and store compressed size
      const Uint before = data_stream.tellp();
      data_stream << compressed_stream.rdbuf();
      m_header.compressed_blocksizes.push_back(static_cast<Uint>(data_stream.tellp()) - before);

      // clear current block
      m_current_block.str(std::string());
    }

    CompressedStreamHeader m_header;

    /// File pointer where the compressed sizes start
    Uint m_compressed_sizes_start;

    Uint m_wordsize;

    // Uncompressed data for the block that is being appended to
    std::stringstream m_current_block;

    std::stringstream data_stream;
  };

  // Recursively transform nodes to their parallel counterparts
  void make_pvtu(XmlNode& node)
  {
    std::string pname = std::string("P") + std::string(node.content->name(), node.content->name_size());
    node.set_name(pname.c_str());
    const std::vector<std::string> attribs_to_remove = boost::assign::list_of("format")("offset");
    boost_foreach(const std::string& attr_name, attribs_to_remove)
    {
      rapidxml::xml_attribute<char>* attr  = node.content->first_attribute(attr_name.c_str());
      if(attr)
        node.content->remove_attribute(attr);
    }
    XmlNode child;
    for (child.content = node.content->first_node(); child.is_valid() ; child.content = child.content->next_sibling() )
    {
      make_pvtu(child);
    }
  }

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < VTKXML::Writer, MeshWriter, LibVTKXML> aVTKXMLWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

Writer::Writer( const std::string& name )
: MeshWriter(name)
{
    options().add_option("distributed_files", false)
    .pretty_name("Distributed Files")
    .description("Indicate if the filesystem is local to each note. When true, the pvtu file is written on each node.");
}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Writer::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".vtu");
  extensions.push_back(".pvtu");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void Writer::write()
{
  // Path for the file written by the current node
  URI my_path(m_file_path.path());
  const URI my_dir = my_path.base_path();
  const std::string basename = my_path.base_name();
  my_path = my_dir / (basename + "_P" + to_str(PE::Comm::instance().rank()) + ".vtu");

  XmlDoc doc("1.0", "ISO-8859-1");

  // Root node
  XmlNode vtkfile = doc.add_node("VTKFile");
  vtkfile.set_attribute("type", "UnstructuredGrid");
  vtkfile.set_attribute("version", "0.1");
  vtkfile.set_attribute("byte_order", "LittleEndian");
  vtkfile.set_attribute("compressor", "vtkZLibDataCompressor");

  XmlNode unstructured_grid = vtkfile.add_node("UnstructuredGrid");

  const Field& coords = m_mesh->topology().geometry_fields().coordinates();
  const Uint npoints = coords.size();
  const Uint dim = coords.row_size();

  // map for element types
  std::map<GeoShape::Type,int> etype_map = boost::assign::map_list_of
    (GeoShape::TRIAG,5)
    (GeoShape::QUAD, 9)
    (GeoShape::TETRA, 10)
    (GeoShape::HEXA, 12);

  // Count number of elements
  Uint nb_elems = 0;
  Uint nb_conn_nodes = 0;
  boost_foreach(const Elements& elements, find_components_recursively<Elements>(m_mesh->topology()) )
  {
    if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
    {
      const Uint n_elems = elements.size();
      nb_elems += n_elems;
      nb_conn_nodes += n_elems * elements.element_type().nb_nodes();
    }
  }

  XmlNode piece = unstructured_grid.add_node("Piece");
  piece.set_attribute("NumberOfPoints", to_str(npoints));
  piece.set_attribute("NumberOfCells", to_str(nb_elems));

  // Points output
  detail::CompressedStream appended_data;

  XmlNode points_data = piece.add_node("Points").add_node("DataArray");
  points_data.set_attribute("type", sizeof(Real) == 4 ? "Float32" : "Float64");
  points_data.set_attribute("NumberOfComponents", "3");
  points_data.set_attribute("format", "appended");
  points_data.set_attribute("offset", to_str(appended_data.offset()));

  appended_data.start_array(3*npoints, sizeof(Real));
  for(Uint i = 0; i != npoints; ++i)
  {
    const Field::ConstRow row = coords[i];
    for(Uint j = 0; j != dim; ++j)
      appended_data.push_back(row[j]);
    if(dim == 2) appended_data.push_back(Real(0.));
  }
  appended_data.finish_array();

  XmlNode cells = piece.add_node("Cells");

  // Write connectivity
  XmlNode connectivity = cells.add_node("DataArray");
  connectivity.set_attribute("type", "UInt32");
  connectivity.set_attribute("Name", "connectivity");
  connectivity.set_attribute("format", "appended");
  connectivity.set_attribute("offset", to_str(appended_data.offset()));
  appended_data.start_array(nb_conn_nodes, 4);
  boost_foreach(const Elements& elements, find_components_recursively<Elements>(m_mesh->topology()) )
  {
    if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
    {
      const Uint n_elems = elements.size();
      const Connectivity& conn_table = elements.geometry_space().connectivity();
      const Uint n_el_nodes = elements.element_type().nb_nodes();
      for(Uint i = 0; i != n_elems; ++i)
      {
        const Connectivity::ConstRow row = conn_table[i];
        for(Uint j = 0; j != n_el_nodes; ++j)
          appended_data.push_back(static_cast<boost::uint32_t>(row[j]));
      }
    }
  }
  appended_data.finish_array();

  // Write the offsets
  XmlNode offsets = cells.add_node("DataArray");
  offsets.set_attribute("type", "UInt32");
  offsets.set_attribute("Name", "offsets");
  offsets.set_attribute("format", "appended");
  offsets.set_attribute("offset", to_str(appended_data.offset()));
  boost::uint32_t offset = 0;
  appended_data.start_array(nb_elems, 4);
  boost_foreach(const Elements& elements, find_components_recursively<Elements>(m_mesh->topology()) )
  {
    if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
    {
      const Uint n_elems = elements.size();
      const Uint n_el_nodes = elements.element_type().nb_nodes();
      for(Uint i = 0; i != n_elems; ++i)
      {
        offset += n_el_nodes;
        appended_data.push_back(offset);
      }
    }
  }
  appended_data.finish_array();

  XmlNode types = cells.add_node("DataArray");
  types.set_attribute("type", "UInt8");
  types.set_attribute("Name", "types");
  types.set_attribute("format", "appended");
  types.set_attribute("offset", to_str(appended_data.offset()));
  appended_data.start_array(nb_elems, 1);
  boost_foreach(const Elements& elements, find_components_recursively<Elements>(m_mesh->topology()) )
  {
    if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
    {
      const Uint n_elems = elements.size();
      const boost::uint8_t vtk_e_type = etype_map[elements.element_type().shape()];
      for(Uint i = 0; i != n_elems; ++i)
      {
        appended_data.push_back(vtk_e_type);
      }
    }
  }
  appended_data.finish_array();


  XmlNode cell_data = piece.add_node("CellData");
  XmlNode point_data = piece.add_node("PointData");

  std::stringstream data_header( std::ios_base::in | std::ios_base::out | std::ios_base::binary );

  std::set<std::string> added_fields;
  boost_foreach(Handle<Field const> field_ptr, m_fields)
  {
    const Field& field = *field_ptr;

    if(!added_fields.insert(field.uri().string()).second)
      continue;

    for(Uint var_idx = 0; var_idx != field.nb_vars(); ++var_idx)
    {
      const std::string var_name = field.var_name(var_idx);
      const Uint var_begin = field.var_index(var_name);
      const Uint field_size = field.continuous() ? field.size() : nb_elems;
      const Uint var_size = field.var_length(var_idx);
      const Uint var_end = var_begin + var_size;

      XmlNode data_array = field.continuous()
        ? point_data.add_node("DataArray")
        : cell_data.add_node("DataArray");

      data_array.set_attribute("type", sizeof(Real) == 4 ? "Float32" : "Float64");
      data_array.set_attribute("NumberOfComponents", to_str(var_size == 2 && dim == 2 ? 3 : var_size));
      data_array.set_attribute("Name", var_name);
      data_array.set_attribute("format", "appended");
      data_array.set_attribute("offset", to_str(appended_data.offset()));

      appended_data.start_array(field_size*(var_size == 2 && dim == 2 ? 3 : var_size), sizeof(Real));

      if(field.continuous())
      {
        if(dim == 2 && var_size == 2)
        {
          for(Uint i = 0; i != field_size; ++i)
          {
            for(Uint j = var_begin; j != var_end; ++j)
            {
              appended_data.push_back(field[i][j]);
            }
            appended_data.push_back(0.);
          }
        }
        else
        {
          for(Uint i = 0; i != field_size; ++i)
            for(Uint j = var_begin; j != var_end; ++j)
              appended_data.push_back(field[i][j]);
        }
      }
      else
      {
        boost_foreach(const Elements& elements, find_components_recursively<Elements>(m_mesh->topology()) )
        {
          const Connectivity& field_connectivity = field.dict().space(elements).connectivity();
          if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
          {
            const Uint n_elems = elements.size();
            if(dim == 2 && var_size == 2)
            {
              for(Uint i = 0; i != n_elems; ++i)
              {
                for(Uint j = var_begin; j != var_end; ++j)
                {
                  /// @bug the field values of the space should be interpolated to the cell-centre, similar to the tecplot writer
                  appended_data.push_back(field[field_connectivity[i][0]][j]);
                }
                appended_data.push_back(0.);
              }
            }
            else
            {
              for(Uint i = 0; i != n_elems; ++i)
              {
                for(Uint j = var_begin; j != var_end; ++j)
                {
                  /// @bug the field values of the space should be interpolated to the cell-centre, similar to the tecplot writer
                  appended_data.push_back(field[field_connectivity[i][0]][j]);
                }
              }
            }
          }
        }
      }

      appended_data.finish_array();
    }
  }

  // Write to file, inserting the binary data at the end
  std::cout << "writing file " << my_path.path() << std::endl;
  boost::filesystem::fstream fout(my_path.path(), std::ios_base::out | std::ios_base::binary);

  // Remove the closing tag
  std::string xml_string;
  to_string(doc, xml_string);
  boost::algorithm::erase_last(xml_string, "</VTKFile>");
  boost::algorithm::trim_right(xml_string);

  // Write XML meta data
  fout << xml_string;

  // Append  compressed data
  fout << "\n<AppendedData encoding=\"raw\">\n";
  fout << appended_data.data_stream.rdbuf();
  fout << "\n</AppendedData>\n</VTKFile>\n";

  fout.close();

  // Write the parallel header, if needed
  if(PE::Comm::instance().rank() == 0 || options().option("distributed_files").value<bool>())
  {
    URI pvtu_path = my_dir / (basename + ".pvtu");

    XmlDoc pvtu_doc("1.0", "ISO-8859-1");

    // Root node
    XmlNode pvtkfile = pvtu_doc.add_node("VTKFile");
    pvtkfile.set_attribute("type", "PUnstructuredGrid");
    pvtkfile.set_attribute("version", "0.1");
    pvtkfile.set_attribute("byte_order", "LittleEndian");

    XmlNode punstruc = pvtkfile.add_node("UnstructuredGrid");
    piece.deep_copy(punstruc);
    punstruc.content->remove_all_attributes();
    punstruc.set_name("UnstructuredGrid");
    detail::make_pvtu(punstruc);
    punstruc.set_attribute("GhostLevel", "0");

    for(Uint i = 0; i != PE::Comm::instance().size(); ++i)
    {
      const std::string piece_path = basename + "_P" + to_str(i) + ".vtu";
      punstruc.add_node("Piece").set_attribute("Source", piece_path);
    }

    to_file(pvtu_doc, pvtu_path);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // VTKXML
} // mesh
} // cf3
