#ifndef CF_Mesh_CGNS_CWriter_hpp
#define CF_Mesh_CGNS_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshWriter.hpp"

#include "Mesh/CGNS/CGNSAPI.hpp"
#include "Mesh/CGNS/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {

//////////////////////////////////////////////////////////////////////////////

/// This class defines CGNS mesh format writer
/// @author Willem Deconinck
class CGNS_API CWriter : public CMeshWriter, public Shared
{

public: // typedefs

  typedef boost::shared_ptr<CWriter> Ptr;
  typedef boost::shared_ptr<CWriter const> ConstPtr;
  
private : // typedefs
  

public: // functions
  
  /// constructor
  CWriter( const CName& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CWriter"; }

  static void defineConfigOptions ( CF::Common::OptionList& options ) {}

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path);

  virtual std::string get_format() { return "CGNS"; }

  virtual std::vector<std::string> get_extensions();

private: // functions
  

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data
  
  std::string m_fileBasename;
    
}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // namespace CGNS
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CGNS_CWriter_hpp
