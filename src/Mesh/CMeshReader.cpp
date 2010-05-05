#include "Common/Log.hpp"
#include "Common/Factory.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/MeshReader.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshReader::CMeshReader ( const CName& name  ) :
  Component ( name )
{
}

////////////////////////////////////////////////////////////////////////////////

CMeshReader::~CMeshReader()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::set_reader(const std::string& reader_name)
{
  Common::SafePtr< MeshReader::PROVIDER > prov =
     Factory<MeshReader>::getInstance().getProvider( reader_name );

  m_meshReader = prov->create();
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
