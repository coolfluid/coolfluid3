#ifndef CF_Mesh_CTable_hpp
#define CF_Mesh_CTable_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostArray.hpp"
#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"
#include "Mesh/BufferT.hpp"

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
  typedef boost::shared_ptr<CTable> Ptr;
  typedef boost::shared_ptr<CTable const> ConstPtr;
  typedef boost::multi_array<Uint,2> ConnectivityTable;
  typedef ConnectivityTable::subarray<1>::type Row;
  typedef ConnectivityTable::const_subarray<1>::type ConstRow;
  typedef BufferT<Uint> Buffer;
  
  /// Contructor
  /// @param name of the component
  CTable ( const CName& name );

  /// Virtual destructor
  virtual ~CTable();

  /// Get the class name
  static std::string getClassName () { return "CTable"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  // functions specific to the CTable component
    
  /// Initialize the connectivity array with a fixed column size
  void initialize(const Uint nbCols);
  
  /// @return A mutable reference to the connectivity table data
  ConnectivityTable& get_table() { return m_table; }
  
  /// @return A const reference to the connectivity table data
  const ConnectivityTable& get_table() const { return m_table; }

  /// @return A Buffer object that can fill this Connectivity Table
  Buffer create_buffer(const size_t buffersize=1024);

  /// @return A mutable row of the underlying array
  Row operator[](const Uint idx) { return m_table[idx]; }

  /// @return A const row of the underlying array
  ConstRow operator[](const Uint idx) const { return m_table[idx]; }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data
  
  ConnectivityTable m_table;  
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTable_hpp
