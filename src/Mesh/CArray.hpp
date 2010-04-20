#ifndef CF_Mesh_CArray_HH
#define CF_Mesh_CArray_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Mesh component class
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CArray : public Common::Component {

public:

  /// Contructor
  /// @param name of the component
  CArray ( const CName& name );

  /// Virtual destructor
  virtual ~CArray();

  /// Get the class name
  static std::string getClassName () { return "CArray"; }

  // functions specific to the CArray component
  
private:

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CArray_HH
