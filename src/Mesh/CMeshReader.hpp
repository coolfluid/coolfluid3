#ifndef CF_Mesh_CMeshReader_hpp
#define CF_Mesh_CMeshReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

  class MeshReader;
  
////////////////////////////////////////////////////////////////////////////////

/// CMeshReader component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API CMeshReader : public Common::Component {

public:

  /// Contructor
  /// @param name of the component
  CMeshReader ( const CName& name );

  /// Virtual destructor
  virtual ~CMeshReader();

  /// Get the class name
  static std::string getClassName () { return "CMeshReader"; }

  // functions specific to the CMeshReader component
  
  /// set the mesh reader
  void set_reader(const std::string& reader_name);
  
private:
    
  boost::shared_ptr<MeshReader> m_meshReader;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshReader_hpp
