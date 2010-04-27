#ifndef CF_Mesh_CArray_HH
#define CF_Mesh_CArray_HH

////////////////////////////////////////////////////////////////////////////////

#define BOOST_MULTI_ARRAY_NO_GENERATORS false
#include "boost/multi_array.hpp" 
#undef BOOST_MULTI_ARRAY_NO_GENERATORS

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

#include "Mesh/Buffer.hpp"

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

/// Array component class
/// This class can store an array
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CArray : public Common::Component {

public:

    typedef boost::multi_array<Real,2> Array;
    typedef Array::subarray<1>::type Row;  

    /// Contructor
    /// @param name of the component
    CArray ( const CName& name );

    /// Virtual destructor
    virtual ~CArray();

    /// Get the class name
    static std::string getClassName () { return "CArray"; }

    // functions specific to the CArray component

    /// Initialize the connectivity array
    /// This will set the column size and allocate a buffer
    void initialize(const Uint nbCols);

    Array& get_array() { return m_array; }

    Buffer<Array>& get_buffer() { return m_buffer; }


  /// private data
  private:

    Array m_array;
    Buffer<Array> m_buffer;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CArray_HH
