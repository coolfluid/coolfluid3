#ifndef CF_Mesh_CTable_HH
#define CF_Mesh_CTable_HH

////////////////////////////////////////////////////////////////////////////////

#define BOOST_MULTI_ARRAY_NO_GENERATORS false
#include "boost/multi_array.hpp" 
#undef BOOST_MULTI_ARRAY_NO_GENERATORS

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"
#include "Mesh/Buffer.hpp"

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
  
  typedef boost::multi_array<Uint,2> ConnectivityTable;
  typedef ConnectivityTable::subarray<1>::type ConnRow;  
  
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
  void initialize(const Uint nbCols);
  
  ConnectivityTable& get_table() { return m_table; }
  
  Buffer<ConnectivityTable>& get_buffer() { return m_buffer; }
  
  
/// private data
private:
  
  ConnectivityTable m_table;
  Buffer<ConnectivityTable> m_buffer;
  

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTable_HH
