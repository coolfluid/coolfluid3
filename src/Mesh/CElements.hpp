#ifndef CF_Mesh_CElements_HH
#define CF_Mesh_CElements_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

  class ElementType;

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
  boost::shared_ptr<ElementType>& get_elementType() { return m_elementType; }

  /// set the element type
  void set_elementType(const std::string& etype_name);
  
private:
  
  boost::shared_ptr<ElementType> m_elementType;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CElements_HH
