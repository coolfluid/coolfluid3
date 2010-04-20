#ifndef CF_Mesh_CTable_HH
#define CF_Mesh_CTable_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"
#define BOOST_MULTI_ARRAY_NO_GENERATORS false
#include "boost/multi_array.hpp" 
#undef BOOST_MULTI_ARRAY_NO_GENERATORS

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

  /// Contructor
  /// @param name of the component
  CTable ( const CName& name );

  /// Virtual destructor
  virtual ~CTable();

  /// Get the class name
  static std::string getClassName () { return "CTable"; }

  // functions specific to the CTable component

  /// Initialize the connectivity table
  void initialize(const Uint& cols, const Uint& buffersize);
  
  /// Change the buffer to the new size
  void change_buffersize(const Uint& buffersize);
  
  /// Clear the table
  void clear();
  
  /// Mutator for table elements
  Uint& operator() (Uint iRow, Uint jCol)
  {
    cf_assert(iRow < m_nbRows);
    cf_assert(jCol < m_nbCols);
    return m_table[iRow][jCol];
  }

  /// Accessor for table elements
  Uint operator() (Uint iRow, Uint jCol) const
  {
    cf_assert(iRow < m_nbRows);
    cf_assert(jCol < m_nbCols);
    return m_table[iRow][jCol];
  }
  
  /// flush the buffer in the connectivity table
  void flush();
  
  /// add a row to the buffer. If the buffer is full, 
  /// it is flushed into the connectivity table
  void add_row(const std::vector<Uint>& row);
  
  /// Set the row to the given values from the table
  void set_row(Uint iRow, std::vector<Uint>& row) const;
  
  /// Get the total number of entries in the table
  Uint size() const { return m_nbRows*m_nbCols;}
  
  /// Get the number of rows
  Uint nbRows() const {return m_nbRows;}

  /// Get the number of columns
  Uint nbCols() const {return m_nbCols;}
  

/// private functions
private:
  
  /// set the dimensions of the buffer
  void create_buffer(const Uint& rows, const Uint& cols);
  
/// private data
private:
  
  /// definition of the array type
  typedef boost::multi_array<Uint, 2> Table;
    
  /// number of rows grows automatically as rows are added
  Uint m_nbRows;
  
  /// number of columns is fixed by user
  Uint m_nbCols;
  
  /// storage of the connectivity table
  Table m_table;
  
  /// number of filled rows in the buffer
  Uint m_buffersize;
  
  /// number of rows allocated in the buffer
  Uint m_maxBuffersize;
  
  /// the buffer to be written in the connectivity table
  Table m_buffer;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTable_HH
