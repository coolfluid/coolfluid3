// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/restrict.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/Signal.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/BinaryDataReader.hpp"
#include "common/FindComponents.hpp"

#include "common/PE/Comm.hpp"

#include "common/XML/FileOperations.hpp"
#include "common/XML/XmlNode.hpp"

#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

struct BinaryDataReader::Implementation
{
  Implementation(const URI& file, const Uint rank) :
    xml_doc(XML::parse_file(file)),
    m_rank(rank)
  {
    XmlNode cfbinary(xml_doc->content->first_node("cfbinary"));
    cf3_assert(from_str<Uint>(cfbinary.attribute_value("version")) == version());

    XmlNode nodes(cfbinary.content->first_node(("nodes")));
    XmlNode node(nodes.content->first_node("node"));
    for(; node.is_valid(); node = XmlNode(node.content->next_sibling("node")))
    {
      const Uint found_rank = from_str<Uint>(node.attribute_value("rank"));
      if(found_rank != m_rank)
        continue;

      const std::string binary_file_name = node.attribute_value("filename");

      binary_file.open(binary_file_name, std::ios_base::in | std::ios_base::binary);
      my_node = node;
    }

    if(!my_node.is_valid())
      throw SetupError(FromHere(), "No node found for rank " + to_str(m_rank));
  }

  ~Implementation()
  {
  }

  Uint version() const
  {
    static const Uint current_version = 1;
    return current_version;
  }
  
  XmlNode get_block_node(const Uint block_idx)
  {
    XmlNode block_node(my_node.content->first_node("block"));
    for(; block_node.is_valid(); block_node = XmlNode(block_node.content->next_sibling("block")))
    {
      if(from_str<Uint>(block_node.attribute_value("index")) == block_idx)
        return block_node;
    }
    
    throw SetupError(FromHere(), "Block with index " + to_str(block_idx) + " was not found");
  }

  void read_data_block(char *data, const Uint count, const Uint block_idx)
  {
    static const std::string block_prefix("__CFDATA_BEGIN");
    
    XmlNode block_node = get_block_node(block_idx);
      
    const Uint block_begin = from_str<Uint>(block_node.attribute_value("begin"));
    const Uint block_end = from_str<Uint>(block_node.attribute_value("end"));
    const Uint compressed_size = block_end - block_begin - block_prefix.size();

    // Check the prefix
    binary_file.seekg(block_begin);
    std::vector<char> prefix_buf(block_prefix.size());
    binary_file.read(&prefix_buf[0], block_prefix.size());
    const std::string read_prefix(prefix_buf.begin(), prefix_buf.end());
    if(read_prefix != block_prefix)
      throw SetupError(FromHere(), "Bad block prefix for block " + to_str(block_idx));
   
    if(count != 0)
    {
      // Build a decompressing stream
      boost::iostreams::filtering_istream decompressing_stream;
      decompressing_stream.set_auto_close(false);
      decompressing_stream.push(boost::iostreams::zlib_decompressor());
      decompressing_stream.push(boost::iostreams::restrict(binary_file, 0, compressed_size));
      
      // Read the data
      decompressing_stream.read(data, count);
      decompressing_stream.pop();
    }
    
    cf3_assert(binary_file.tellg() == block_end);
  }

  // XML document describing all data added
  boost::shared_ptr<XmlDoc> xml_doc;

  // Binary file
  boost::filesystem::fstream binary_file;

  // Xml data for the blocks associated with the current rank
  XmlNode my_node;

  // Rank to read
  const Uint m_rank;
};
  
////////////////////////////////////////////////////////////////////////////////////////////

BinaryDataReader::BinaryDataReader ( const std::string& name ) : Component(name)
{
  options().add("file", URI())
    .pretty_name("File")
    .description("File name for the output file")
    .attach_trigger(boost::bind(&BinaryDataReader::trigger_file, this));

  options().add("rank", common::PE::Comm::instance().rank())
    .pretty_name("Rank")
    .description("Rank for which to read data")
    .attach_trigger(boost::bind(&BinaryDataReader::trigger_file, this));
}

BinaryDataReader::~BinaryDataReader()
{
  close();
}

void BinaryDataReader::close()
{
  m_implementation.reset();
}

Uint BinaryDataReader::block_cols ( const Uint block_idx )
{
  return from_str<Uint>(m_implementation->get_block_node(block_idx).attribute_value("nb_cols"));
}

Uint BinaryDataReader::block_rows ( const Uint block_idx )
{
  return from_str<Uint>(m_implementation->get_block_node(block_idx).attribute_value("nb_rows"));
}

std::string BinaryDataReader::block_name ( const Uint block_idx )
{
  return m_implementation->get_block_node(block_idx).attribute_value("name");
}

std::string BinaryDataReader::block_type_name ( const Uint block_idx )
{
  return m_implementation->get_block_node(block_idx).attribute_value("type_name");
}


void BinaryDataReader::read_data_block(char *data, const Uint count, const Uint block_idx)
{
  if(is_null(m_implementation.get()))
    throw SetupError(FromHere(), "No open file for BinaryDataReader at " + uri().path());
  
  m_implementation->read_data_block(data, count, block_idx);
}

void BinaryDataReader::trigger_file()
{
  const URI file_uri = options().value<URI>("file");
  if(!boost::filesystem::exists(file_uri.path()))
  {
    throw SetupError(FromHere(), "Input file " + file_uri.path() + " does not exist");
  }
  m_implementation.reset(new Implementation(file_uri, options().value<Uint>("rank")));
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
