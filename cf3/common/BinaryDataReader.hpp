// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_BinaryDataReader_hpp
#define cf3_common_BinaryDataReader_hpp

#include <boost/scoped_ptr.hpp>

#include "common/Component.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  
///////////////////////////////////////////////////////////////////////////////////////

  
/// Component for writing binary data collected into a single file
class Common_API BinaryDataReader : public Component {

public: // functions

  /// Contructor
  /// @param name of the component
  BinaryDataReader ( const std::string& name );
  ~BinaryDataReader();

  /// Get the class name
  static std::string type_name () { return "BinaryDataReader"; }

  /// Read the given block into the supplied table. The table is resized as needed
  template<typename T>
  void read_table(Table<T>& table, const Uint block_idx)
  {
    const Uint rows = block_rows(block_idx);
    const Uint cols = block_cols(block_idx);
    table.set_row_size(cols);
    table.resize(rows);
    read_data_block(reinterpret_cast<char*>(table.array().data()), sizeof(T)*rows*cols, block_idx);
  }
  
  /// Read the given block into the supplied list. The list is resized as needed
  template<typename T>
  void read_list(List<T>& list, const Uint block_idx)
  {
    const Uint rows = block_rows(block_idx);
    list.resize(rows);
    read_data_block(reinterpret_cast<char*>(list.array().data()), sizeof(T)*rows, block_idx);
  }

  /// Close the current file
  void close();

  /// Number of rows for the given block
  Uint block_rows(const Uint block_idx);

  /// Number of columns for the given block
  Uint block_cols(const Uint block_idx);

  /// Name of the given block
  std::string block_name(const Uint block_idx);

private:
  // Read aata block from the binary file
  void read_data_block(char* data, const Uint count, const Uint block_idx);

  // Trigger on output file change
  void trigger_file();

  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_BinaryDataReader_hpp
