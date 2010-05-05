#ifndef CF_Mesh_MeshReader_hpp
#define CF_Mesh_MeshReader_hpp

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

/// This class represents the the data related to an MeshReader
/// @author Willem Deconinck
class Mesh_API MeshReader {

public: // functions

  typedef Common::ConcreteProvider < MeshReader > PROVIDER;

  /// Default constructor without arguments
  MeshReader();

  /// Default destructor
  ~MeshReader();

  static std::string getClassName() { return "MeshReader"; }

public: // accessors

  void set_mesh(const boost::shared_ptr<CMesh>& mesh) { m_mesh = mesh; }

  void read(boost::filesystem::path& fp, const boost::shared_ptr<CMesh>& mesh)
  {    
    set_mesh(mesh);    
        
    // if the file is present open it
    boost::filesystem::fstream file;
    if( boost::filesystem::exists(fp) )
    {
       CFLog(VERBOSE, "Opening file " <<  fp.string() << "\n");
       file.open(fp,std::ios_base::in); // exists so open it
    }
    else // doesnt exist so throw exception
    {
       throw boost::filesystem::filesystem_error( fp.string() + " does not exist",
                                                  boost::system::error_code() );
    }    
    read_impl(file);
    file.close();
  }
  
  virtual void read_impl(std::fstream& file) = 0;

  
protected: // data

  boost::shared_ptr<CMesh> m_mesh;
  
}; // end of class MeshReader
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_MeshReader_hpp
