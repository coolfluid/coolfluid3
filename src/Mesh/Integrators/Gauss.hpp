#ifndef CF_Mesh_Integrators_Gauss_hpp
#define CF_Mesh_Integrators_Gauss_hpp

#include <boost/assign/list_of.hpp>

#include "Common/AssertionManager.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/Integrators/GaussImplementation.hpp"
#include "Math/RealVector.hpp"


namespace CF {
namespace Mesh {
namespace Integrators {

/// Perform Gauss integration, using the supplied shape function types
/// Functors are evaluated in mapped coordinates and must include the multiplication
/// with the appropriate Jacobian determinant.
template<typename GeoShapeF, typename SolShapeF=GeoShapeF, Uint Order=1>
class Gauss
{
public:
  template<typename FunctorT, typename ResultT>
  static void integrate(FunctorT& functor, ResultT& result)
  {
    // Delegate to an integration function specific to the shape and required order
    GaussImplementation<GeoShapeF::shape, SolShapeF::shape, Order>::integrate(functor, result);
  }
};

} // namespace Gauss
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_Integrators_Gauss_hpp */
