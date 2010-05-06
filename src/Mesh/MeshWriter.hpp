#ifndef CF_Mesh_MeshWriter_hpp
#define CF_Mesh_MeshWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Mesh/MeshAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;

  class CMesh;
  
////////////////////////////////////////////////////////////////////////////////

/// This class represents an abstract interface to a MeshWriter
/// @author Willem Deconinck
class Mesh_API MeshWriter {

public: // functions

  typedef Common::ConcreteProvider < MeshWriter > PROVIDER;

  /// Default constructor without arguments
  MeshWriter();

  /// Default destructor
  ~MeshWriter();

  static std::string getClassName() { return "MeshWriter"; }

public: // accessors

  void write(const boost::shared_ptr<CMesh>& mesh, boost::filesystem::path& fp)
  {    
    set_mesh(mesh);    
        
    // if the file is present open it
    boost::filesystem::fstream file;
    CFLog(VERBOSE, "Opening file " <<  fp.string() << "\n");
    file.open(fp,std::ios_base::out);
    if (!file) // didn't open so throw exception
    {
       throw boost::filesystem::filesystem_error( fp.string() + " failed to open",
                                                  boost::system::error_code() );
    }
    write_impl(file);
    file.close();
  }
  
  virtual void write_impl(std::fstream& file) = 0;

private:
  void set_mesh(const boost::shared_ptr<CMesh>& mesh) { m_mesh = mesh; }
  
protected: // data

  boost::shared_ptr<CMesh> m_mesh;
  
}; // end of class MeshWriter
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_MeshWriter_hpp
