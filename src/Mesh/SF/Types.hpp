#ifndef CF_Mesh_SF_Types_hpp
#define CF_Mesh_SF_Types_hpp

#include <boost/mpl/vector.hpp>

#include "Line1DLagrangeP1.hpp"
#include "Triag2DLagrangeP1.hpp"
#include "Quad2DLagrangeP1.hpp"
#include "Tetra3DLagrangeP1.hpp"
#include "Hexa3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// List of all supported shapefunctions
typedef boost::mpl::vector< Line1DLagrangeP1,
                            Triag2DLagrangeP1,
                            Quad2DLagrangeP1,
                            Hexa3DLagrangeP1,
                            Tetra3DLagrangeP1
> Types;

} // namespace LagrangeSF
} // namespace Mesh
} // namespace CF

#endif // CF_Mesh_SF_Types_hpp
