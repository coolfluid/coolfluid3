#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::CMeshTransformer ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::~CMeshTransformer()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::defineConfigOptions(Common::OptionList& options)
{
  //options.add< OptionT<std::string> >  ( "File",  "File to read" , "" );
  //options.add< Common::OptionT<std::string> >  ( "Mesh",  "Mesh to construct" , "" );
}

//////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::transform( XmlNode& node  )
{
  // Get the mesh component in the tree
  /// @todo[1]: wait for Tiago for functionality

  // Get the file path
  // boost::filesystem::path file = option("File")->value<std::string>();

  // Call implementation
  /// @todo wait for todo[1]
  // read_from_to(file,mesh);

}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
