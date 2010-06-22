#ifndef CF_Mesh_LagrangeSF_SFTypes_hpp
#define CF_Mesh_LagrangeSF_SFTypes_hpp

#include <boost/mpl/vector.hpp>

#include "Mesh/LagrangeSF/TriagP1.hpp"
#include "Mesh/LagrangeSF/QuadP1.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeSF {

/// List of all supported shapefunctions
typedef boost::mpl::vector< TriagP1, QuadP1 > SFTypes;

} // namespace LagrangeSF
} // namespace Mesh
} // namespace CF

#endif // CF_Mesh_LagrangeSF_SFTypes_hpp
