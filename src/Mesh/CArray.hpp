#ifndef CF_Mesh_CArray_hpp
#define CF_Mesh_CArray_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostArray.hpp"
#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

#include "Mesh/BufferT.hpp"

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

/// Array component class
/// This class can store an array
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CArray : public Common::Component {

public: // typedefs

  typedef Common::Component_iterator<CArray> Iterator;
  typedef boost::shared_ptr<CArray> Ptr;
  typedef boost::multi_array<Real,2> Array;
  typedef Array::subarray<1>::type Row;  
  typedef Array::const_subarray<1>::type ConstRow;
  typedef BufferT<Real> Buffer;

public: // functions

  /// Contructor
  /// @param name of the component
  CArray ( const CName& name );

  /// Virtual destructor
  virtual ~CArray();

  /// Get the class name
  static std::string getClassName () { return "CArray"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  // functions specific to the CArray component

  /// Initialize the array with a fixed column size
  void initialize(const Uint nbCols);

  /// @return A reference to the array data
  Array& get_array() { return m_array; }

  /// @return A const reference to the array data
  const Array& get_array() const { return m_array; }

  /// @return A Buffer object that can fill this Array
  Buffer create_buffer(const size_t buffersize=1024);

private: // data

  /// storage of the array
  Array m_array;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CArray_hpp
