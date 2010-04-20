#ifndef CF_Mesh_CTable_HH
#define CF_Mesh_CTable_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component holding a connectivity table
/// @author Tiago Quintino
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

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTable_HH
