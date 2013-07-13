// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_BinaryDataWriter_hpp
#define cf3_common_BinaryDataWriter_hpp

#include <boost/scoped_ptr.hpp>

#include "common/Component.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  
///////////////////////////////////////////////////////////////////////////////////////

  
/// Component for writing binary data collected into a single file
class Common_API BinaryDataWriter : public Component {

public: // functions

  /// Contructor
  /// @param name of the component
  BinaryDataWriter ( const std::string& name );
  ~BinaryDataWriter();

  /// Get the class name
  static std::string type_name () { return "BinaryDataWriter"; }

  /// Append a new data block containing data from the supplied table. An index into the current file is returned
  template<typename T>
  Uint append_data(const Table<T>& table)
  {
    return write_data_block(reinterpret_cast<const char*>(table.array().data()), sizeof(T)*table.row_size()*table.size(), table.name(), table.size(), table.row_size());
  }
  
  /// Append a new data block, returning the block index number for the current file
  template<typename T>
  Uint append_data(const List<T>& list)
  {
    return write_data_block(reinterpret_cast<const char*>(list.array().data()), sizeof(T)*list.size(), list.name(), list.size(), 1);
  }

  /// Close the current file
  void close();

private:
  // Write a data block to the binary file
  Uint write_data_block(const char* data, const std::streamsize count, const std::string& list_name, const Uint nb_rows, const Uint nb_cols);

  // Trigger on output file change
  void trigger_file();

  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_BinaryDataWriter_hpp
