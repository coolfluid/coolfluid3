#include "MeshGeneration.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"


using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Testing {
  
////////////////////////////////////////////////////////////////////////////////

void create_rectangle(CMesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments)
{
  const Uint dim = 2;
  CArray& coordinates = *mesh.create_component_type<CArray>("coordinates");
  coordinates.initialize(dim);
  CArray::Array& coordArray = coordinates.array();
  coordArray.resize(boost::extents[(x_segments+1)*(y_segments+1)][dim]);
  const Real x_step = x_len / static_cast<Real>(x_segments);
  const Real y_step = y_len / static_cast<Real>(y_segments);
  Real y;
  for(Uint j = 0; j <= y_segments; ++j)
  {
    y = static_cast<Real>(j) * y_step;
    for(Uint i = 0; i <= x_segments; ++i)
    {
      CArray::Row row = coordArray[j*(x_segments+1)+i];
      row[XX] = static_cast<Real>(i) * x_step;
      row[YY] = y;
    }
  }
  CRegion& region = mesh.create_region("region");
  CTable::ConnectivityTable& connArray = region.create_elements("Quad2DLagrangeP1",coordinates).connectivity_table().table();
  connArray.resize(boost::extents[(x_segments)*(y_segments)][4]);
  for(Uint j = 0; j < y_segments; ++j)
  {
    for(Uint i = 0; i < x_segments; ++i)
    {
      CTable::Row nodes = connArray[j*(x_segments)+i];
      nodes[0] = j * (x_segments+1) + i;
      nodes[1] = nodes[0] + 1;
      nodes[3] = (j+1) * (x_segments+1) + i;
      nodes[2] = nodes[3] + 1;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////


