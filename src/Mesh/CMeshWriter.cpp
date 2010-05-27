#include "Common/OptionT.hpp"

#include "Mesh/CMeshWriter.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshWriter::CMeshWriter ( const CName& name  ) :
  Component ( name )
{
  build_component(this);
}

////////////////////////////////////////////////////////////////////////////////

CMeshWriter::~CMeshWriter()
{
}

////////////////////////////////////////////////////////////////////////////////


void CMeshWriter::defineConfigOptions(Common::OptionList& options)
{
  options.add< OptionT<std::string> >  ( "File",  "File to read" , "" );
  //options.add< Common::OptionT<std::string> >  ( "Mesh",  "Mesh to construct" , "" );
}

//////////////////////////////////////////////////////////////////////////////

void CMeshWriter::write( XmlNode& node  )
{
  // Get the mesh component in the tree
  /// @todo[1]: wait for Tiago for functionality

  // Get the file path
  boost::filesystem::path file = option("File")->value<std::string>();

  // Call implementation
  /// @todo wait for todo[1]
  // write_from_to(mesh,file);

}

//////////////////////////////////////////////////////////////////////////////

boost::filesystem::path CMeshWriter::write_from(const CMesh::Ptr& mesh)
{
  // Get the file path
  boost::filesystem::path file = option("File")->value<std::string>();

  // Call implementation
  write_from_to(mesh,file);

  // return the file
  return file;
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
