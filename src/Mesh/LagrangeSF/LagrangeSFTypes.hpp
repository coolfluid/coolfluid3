#ifndef CF_Mesh_LagrangeSF_LagrangeSFTypes_hpp
#define CF_Mesh_LagrangeSF_LagrangeSFTypes_hpp

#include <boost/mpl/vector.hpp>

#include "Mesh/LagrangeSF/TriagP1.hpp"
#include "Mesh/LagrangeSF/QuadP1.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeSF {

/// List of all supported shapefunctions
typedef boost::mpl::vector<TriagP1,QuadP1> LagrangeSFTypes;

} // namespace LagrangeSF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_LagrangeSF_LagrangeSF */
