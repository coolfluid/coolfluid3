#ifndef CF_Mesh_CTable_HH
#define CF_Mesh_CTable_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"
#include "Mesh/Table.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component holding a connectivity table
/// The table has to be filled through a buffer.
/// Before using the table one has to be sure that
/// the buffer is flushed.
/// @author Willem Deconinck Tiago Quintino
class Mesh_API CTable : public Common::Component {

public:

  /// type of the connectivity table
  typedef Table<Uint> ConnectivityTable;
  
  /// subarray or row of connectivity table
  typedef ConnectivityTable::Row Row;
  
  /// Contructor
  /// @param name of the component
  CTable ( const CName& name );

  /// Virtual destructor
  virtual ~CTable();

  /// Get the class name
  static std::string getClassName () { return "CTable"; }

  // functions specific to the CTable component

  /// Initialize the connectivity table
  /// This will set the column size and allocate a buffer
  void initialize(const Uint& cols, const Uint& buffersize = 100) { m_table.initialize(cols,buffersize); }
    
  /// get the row with given index.
  /// @return row by reference (boost::multi_array::subarray&)
  Row get_row(const Uint idx) { return m_table.get_row(idx); }

  /// set a vector to the row of the table
  /// @param [in] idx   row index
  /// @param [out] row   vectortype
  template<typename vectorType>
  void set_row(const Uint idx, vectorType& row) { m_table.set_row(idx,row); }
  
  /// add a row to the table
  /// @param [in] row vectortype
  template<typename vectorType>
  void add_row(const vectorType& row) { m_table.add_row(row); }

  /// flush the buffer in the table
  /// Better use finalize(), which also deallocates the buffer
  void flush() { m_table.flush(); }

  /// flush the buffer in the table
  /// Better use finalize(), which also deallocates the buffer
  void finalize() { m_table.finalize(); }
  
  /// get the table to gain boost::multi_array operators
  ConnectivityTable& getTable() { return m_table; }
  
  /// Get the number of rows
  Uint nbRows() const {return m_table.nbRows(); }

  /// Get the number of columns
  Uint nbCols() const {return m_table.nbCols(); }
  
  
/// private data
private:
  
  ConnectivityTable m_table;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTable_HH
