#include "Common/Log.hpp"
#include "Common/Factory.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/MeshWriter.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshWriter::CMeshWriter ( const CName& name  ) :
  Component ( name )
{
}

////////////////////////////////////////////////////////////////////////////////

CMeshWriter::~CMeshWriter()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshWriter::set_writer(const std::string& writer_name)
{
  Common::SafePtr< MeshWriter::PROVIDER > prov =
     Factory<MeshWriter>::getInstance().getProvider( writer_name );

  m_meshWriter = prov->create();
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
