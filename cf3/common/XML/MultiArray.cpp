// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/as_literal.hpp>
#include <boost/tokenizer.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/Assertions.hpp"
#include "common/BasicExceptions.hpp"
#include "common/StringConversion.hpp"
#include "common/TypeInfo.hpp"

#include "common/Log.hpp"

#include "common/XML/Protocol.hpp"

#include "common/XML/MultiArray.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

////////////////////////////////////////////////////////////////////////////

XmlNode add_multi_array_in( Map & map, const std::string & name,
                            const boost::multi_array<Real, 2> & array,
                            const std::string & delimiter,
                            const std::vector<std::string> & labels )
{
  cf3_assert( map.content.is_valid() );
  cf3_assert( !name.empty())
  cf3_assert( !map.check_entry(name) );
  cf3_assert( !delimiter.empty() );

  std::string labels_str = boost::algorithm::join(labels, delimiter);

  XmlNode array_node =  map.content.add_node( Protocol::Tags::node_array() );

  array_node.add_node( common::class_name<std::string>(), labels_str );

  XmlNode data_node = array_node.add_node( common::class_name<Real>() );

  Uint nb_rows = array.size();
  Uint nb_cols = 0;

  std::string str;
  std::string size;

  if(nb_rows != 0)
    nb_cols = array[0].size();

  size = to_str(nb_rows) + ':' + to_str(nb_cols);

  array_node.set_attribute( Protocol::Tags::attr_key(), name );

  data_node.set_attribute( "dimensions", to_str((Uint)array.dimensionality) );
  data_node.set_attribute( Protocol::Tags::attr_array_delimiter(), delimiter );
  data_node.set_attribute( Protocol::Tags::attr_array_size(), size);
  data_node.set_attribute( "merge_delimiter", to_str(true) ); // temporary

  // build the value string (ideas are welcome to avoid multiple
  // memory reallocations
  for(Uint row = 0 ; row < nb_rows ; ++row)
  {
    for(Uint col = 0 ; col < nb_cols ; ++col)
      str += to_str( array[row][col] ) + delimiter;

    str += '\n'; // line break after each row
  }

  data_node.set_value(str.c_str());

  return array_node;
}

////////////////////////////////////////////////////////////////////////////

void get_multi_array( const Map & map, const std::string & name,
                          boost::multi_array<Real, 2> & array,
                          std::vector<std::string> & labels )
{
  cf3_assert( map.content.is_valid() );
  cf3_assert( !name.empty());

  // make the code a bit more readable
  using namespace boost;            // first_finder() and is_iequal()
  using namespace boost::algorithm; // split_iterator and make_split_iterator()
  typedef split_iterator<std::string::iterator> StringSplitIterator;
  typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;

  std::vector<Uint> sizes;
  std::string delimiter;
  rapidxml::xml_attribute<char> * attr = nullptr;

  //
  // 1. Some information gathering and checks
  //

  // 1a. check the array exists
  XmlNode array_node = map.find_value(name, Protocol::Tags::node_array());

  if(!array_node.is_valid())
    throw ValueNotFound(FromHere(), "Could not find a multi-array of name [" + name + "]." );

  // 1b. look for labels and data nodes
  XmlNode labels_node( array_node.content->first_node( common::class_name<std::string>().c_str() ) );
  XmlNode data_node( array_node.content->first_node( common::class_name<Real>().c_str() ) );

  if(!data_node.is_valid())
    throw ValueNotFound(FromHere(), "Could not find data for multi-array [" + name + "]." );

  // 1c. get the delimiter
  attr = data_node.content->first_attribute( Protocol::Tags::attr_array_delimiter() );

  if( is_null(attr) )
    throw XmlError(FromHere(), "Could not find the delimiter for multi-array [" + name + "].");

  delimiter = attr->value();

  // 1d. get the multi-array size and resize the target multi-array
  attr = data_node.content->first_attribute( Protocol::Tags::attr_array_size() );

  if( is_null(attr) )
    throw XmlError(FromHere(), "Could not find the size of multi-array [" + name + "].");

  Map::split_string( attr->value(), ":", sizes, 2 );

  if( sizes.size() != 2 )
    throw XmlError(FromHere(), "The multi-array size ["+ std::string(attr->value()) +"] is not valid.");

  array.resize(boost::extents[ sizes[0] ][ sizes[1] ] );

  // 1e. get the labels
  if( labels_node.is_valid() )
    Map::split_string( labels_node.content->value(), delimiter, labels);

  //
  // 2. Fill the multi-array
  //

  // the array is written in the XML as a 2D array, with a new line after each
  // row. Thus we first need to tokenize the string on line breaks and then
  // split the line depending on the delimiter and cast each element to Real.

  boost::char_separator<char> sep("\n");
  std::string str( data_node.content->value() );
  Tokenizer tokens( str, sep );
  Tokenizer::iterator tok_iter = tokens.begin();

  for (int row = 0 ; tok_iter != tokens.end() && row < sizes[0];  ++tok_iter, ++row)
  {
    std::string value_str(*tok_iter);
    StringSplitIterator it = make_split_iterator(value_str, first_finder(delimiter, is_iequal()));

    for(int col = 0 ; it != StringSplitIterator() && col < sizes[1] ; ++it, ++col )
    {
      // split_iterator works with ranges in the given string. Each range
      // is a portion of the string between 2 delimiters.
      // copy_range() copies the content of this range to a new string (in our case).
      std::string val( boost::copy_range<std::string>(*it) );

      try
      {
        array[row][col] = from_str<Real>(val);
      }
      catch(boost::bad_lexical_cast e)
      {
        throw CastingFailed(FromHere(),  "Enable to cast [" + val + "] to Real.");
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3
