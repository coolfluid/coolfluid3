#ifndef CF_Mesh_Integrators_Gauss_HH
#define CF_Mesh_Integrators_Gauss_HH

#include <boost/assign/list_of.hpp>

#include "Common/AssertionManager.hpp"
#include "Mesh/LagrangeSF/TriagP1.hpp"
#include "Math/RealVector.hpp"

namespace CF {
namespace Mesh {
namespace Integrators {

using namespace CF::Mesh::LagrangeSF;

/// Perform Gauss integration, using the supplied shape function types
/// Functors are evaluated in mapped coordinates and must include the multiplication
/// with the appropriate Jacobian determinant.
template<typename GeoShapeF, typename SolShapeF=GeoShapeF, Uint Order=1>
class Gauss
{
public:
  template<typename FunctorT, typename ResultT>
  static void integrate(FunctorT& functor, ResultT& Result)
  {
    BOOST_STATIC_ASSERT(sizeof(ResultT) == 0); // Break compilation on non-specialized instantiation
  }
};

template<>
class Gauss<TriagP1, TriagP1, 1>
{
public:
  template<typename FunctorT, typename ResultT>
  static void integrate(FunctorT& functor, ResultT& Result)
  {
    static const double mu = 0.3333333333333333333333333;
    static const double w = 0.5;
    static const RealVector mapped_coords = boost::assign::list_of(mu)(mu);
    Result = w * functor(mapped_coords);
  }
};

} // namespace Gauss
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_Integrators_Gauss_HH */
