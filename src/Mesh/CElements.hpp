#ifndef CF_Mesh_CElements_HH
#define CF_Mesh_CElements_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"
#include "ElementType.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// CElements component class
/// This class stores information about elements 
/// to regions (CRegion)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CElements : public Common::Component {

public:

  /// Contructor
  /// @param name of the component
  CElements ( const CName& name );

  /// Virtual destructor
  virtual ~CElements();

  /// Get the class name
  static std::string getClassName () { return "CElements"; }

  // functions specific to the CElements component

  /// return the elementType
  ElementType& get_elementType() { return m_elementType; }

private:
  
  ElementType m_elementType;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CElements_HH
