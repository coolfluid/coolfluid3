#ifndef CF_Mesh_CMeshWriter_hpp
#define CF_Mesh_CMeshWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

  class MeshWriter;
  
////////////////////////////////////////////////////////////////////////////////

/// CMeshWriter component class
/// This class serves as a component that that will write
/// the mesh to a file
/// @author Willem Deconinck
class Mesh_API CMeshWriter : public Common::Component {

public:

  /// Contructor
  /// @param name of the component
  CMeshWriter ( const CName& name );

  /// Virtual destructor
  virtual ~CMeshWriter();

  /// Get the class name
  static std::string getClassName () { return "CMeshWriter"; }

  // functions specific to the CMeshWriter component
  
  /// set the mesh writer
  void set_writer(const std::string& writer_name);
  
  /// set the mesh writer
  boost::shared_ptr<MeshWriter>& get_writer() { return m_meshWriter; }
  
  
private:
    
  boost::shared_ptr<MeshWriter> m_meshWriter;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshWriter_hpp
