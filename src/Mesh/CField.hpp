#ifndef CF_Mesh_CField_HH
#define CF_Mesh_CField_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Mesh component class
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CField : public Common::Component {

public:

  /// Contructor
  /// @param name of the component
  CField ( const CName& name );

  /// Virtual destructor
  virtual ~CField();

  /// Get the class name
  static std::string getClassName () { return "CField"; }

  // functions specific to the CField component
  
private:

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CField_HH
