#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CGNS/CWriter.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Mesh::CGNS::CWriter,
                         Mesh::CMeshWriter,
                         Mesh::CGNS::CGNSLib,
                         1 >
aCGNSWriter_Provider ( "CGNS" );

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const CName& name )
: CMeshWriter(name),
  Shared()
{
  BUILD_COMPONENT;
}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CWriter::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".cgns");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path)
{
  m_mesh = mesh;

  // if the file is present open it
  boost::filesystem::fstream file;
  CFLog(VERBOSE, "Opening file " <<  path.string() << "\n");
  file.open(path,std::ios_base::out);
  if (!file) // didn't open so throw exception
  {
     throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                                boost::system::error_code() );
  }
  m_fileBasename = boost::filesystem::basename(path);

  compute_mesh_specifics();
  
  file.close();

}

/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////


} // CGNS
} // Mesh
} // CF
