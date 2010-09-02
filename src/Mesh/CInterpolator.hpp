#ifndef CF_Mesh_CInterpolator_hpp
#define CF_Mesh_CInterpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/Component.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"

namespace CF {
namespace Mesh {

  class CMesh;
  class CRegion;
  
////////////////////////////////////////////////////////////////////////////////

/// CInterpolator component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API CInterpolator : public Common::Component {

public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < CInterpolator,1 > PROVIDER;
  /// pointer to this type
  typedef boost::shared_ptr<CInterpolator> Ptr;
  typedef boost::shared_ptr<CInterpolator const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CInterpolator ( const CName& name );

  /// Virtual destructor
  virtual ~CInterpolator();

  /// Get the class name
  static std::string type_name () { return "CInterpolator"; }
  
  // --------- Configuration ---------

  static void defineConfigOptions ( Common::OptionList& options );

  // --------- Signals ---------

  void interpolate( Common::XmlNode& node  );

  // --------- Direct access ---------

  virtual void construct_internal_storage(const CMesh::Ptr& source, const CMesh::Ptr& target) = 0;
  
  virtual void interpolate_field_from_to(const CField& source, CField& target) = 0;

protected: // functions

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CInterpolator_hpp
