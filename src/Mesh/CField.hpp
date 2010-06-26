#ifndef CF_Mesh_CField_hpp
#define CF_Mesh_CField_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Field component class
/// This class stores fields which can be applied 
/// to regions (CRegion)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CField : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CField> Ptr;
  typedef boost::shared_ptr<CField const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CField ( const CName& name );

  /// Virtual destructor
  virtual ~CField();

  /// Get the class name
  static std::string getClassName () { return "CField"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  // functions specific to the CField component
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}


};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CField_hpp
