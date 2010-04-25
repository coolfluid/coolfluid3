#ifndef CF_Mesh_CArray_HH
#define CF_Mesh_CArray_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"
#include "Mesh/Table.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Array component class
/// This class can store an array
/// @todo make it with templates
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CArray : public Common::Component {

public:

  /// type of the array
  typedef Table<Real> Array;
  
  /// subarray or row of connectivity table
  typedef Array::Row Row;
  

  /// Contructor
  /// @param name of the component
  CArray ( const CName& name );

  /// Virtual destructor
  virtual ~CArray();

  /// Get the class name
  static std::string getClassName () { return "CArray"; }

  // functions specific to the CArray component
  
  /// Initialize the connectivity table
  /// This will set the column size and allocate a buffer
  void initialize(const Uint& cols, const Uint& buffersize = 1000) { m_array.initialize(cols,buffersize); }
    
  /// get the row with given index.
  /// @return row by reference (boost::multi_array::subarray&)
  Row get_row(const Uint idx) { return m_array.get_row(idx); }

  /// set a vector to the row of the table
  /// @param [in] idx   row index
  /// @param [out] row   vectortype
  template<typename vectorType>
  void set_row(const Uint idx, vectorType& row) { m_array.set_row(idx,row); }
  
  /// add a row to the table
  /// @param [in] row vectortype
  template<typename vectorType>
  void add_row(const vectorType& row) { m_array.add_row(row); }

  /// flush the buffer in the table
  /// Better use finalize(), which also deallocates the buffer
  void flush() { m_array.flush(); }

  /// flush the buffer in the table
  /// Better use finalize(), which also deallocates the buffer
  void finalize() { m_array.finalize(); }
  
  /// get the array to gain boost::multi_array operators
  Array& getArray() { return m_array; }
  
  /// Get the number of rows
  Uint nbRows() const {return m_array.nbRows(); }

  /// Get the number of columns
  Uint nbCols() const {return m_array.nbCols(); }
  
private:
  
  Array m_array;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CArray_HH
