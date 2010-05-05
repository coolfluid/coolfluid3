#include "Common/Log.hpp"
#include "Common/Factory.hpp"

#include "Mesh/CMeshReader.hpp"
//#include "Mesh/MeshReader.hpp"

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

void CMeshReader::set_reader(const std::string& etype_name)
{
  //Common::SafePtr< ElementType::PROVIDER > prov =
  //    Factory<ElementType>::getInstance().getProvider( etype_name );

  //m_elementType = prov->create();
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
