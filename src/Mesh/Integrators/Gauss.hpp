#ifndef CF_Mesh_Integrators_Gauss_hpp
#define CF_Mesh_Integrators_Gauss_hpp

#include <boost/mpl/for_each.hpp>
#include <boost/foreach.hpp>

#include "Common/AssertionManager.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/Integrators/GaussImplementation.hpp"
#include "Mesh/LagrangeSF/SFTypes.hpp"

#include "Math/RealVector.hpp"


namespace CF {
namespace Mesh {
namespace Integrators {

/// Perform Gauss integration, using the supplied shape function types
/// Functors are evaluated in mapped coordinates and must include the multiplication
/// with the appropriate Jacobian determinant in the result.
/// The following function is required:
/// template<typename GeometricShapeFunctionT, typename SolutionShapeFunctionT>
/// ResultT valTimesDetJacobian(const RealVector& mappedCoordinates)
/// This function returns the value of the function to integrate for the given set of mapped coordinates, multiplied with the
/// Jacobian as required.
template<typename GeoShapeF, typename SolShapeF=GeoShapeF, Uint Order=1>
class Gauss
{
public:

  /// Integration over a single element.
  /// The functor should be 'primed' before calling this
  /// so it is aware of the element that is used
  template<typename FunctorT, typename ResultT>
  static void integrateElement(FunctorT& functor, ResultT& result)
  {
    // Delegate to an integration function specific to the shape and required order
    GaussImplementation<GeoShapeF::shape, SolShapeF::shape, Order>::template integrate<GeoShapeF, SolShapeF>(functor, result);
  }

  /// Integration over all elements in a region
  /// This function requires an additional function to be defined for the functor, in order to prime the functor for each element:
  /// void setElement(const Uint element)
  template<typename FunctorT, typename ResultT>
  static void integrateRegion(const CRegion& region, FunctorT& functor, ResultT& result)
  {
    const Uint elem_begin = 0;
    const Uint elem_end = region.elements_count();
    for(Uint elem = elem_begin; elem != elem_end; ++elem)
    {
      functor.setElement(elem); // initialize element-specific functor data
      integrateElement(functor, result);
    }
  }
};

/// Chooses an appropriate integrator, based on the element type of the region
/// and executes the integration if a match is found
template<typename FunctorT, typename ResultT, Uint IntegrationOrder=1>
struct RegionIntegrator
{
  RegionIntegrator(const CRegion& region, FunctorT& functor, ResultT& result, bool& integratorFound)
    : found(integratorFound),
      m_region(region),
      m_functor(functor),
      m_result(result)
  {}

  template<typename ShapeFunctionT> void operator()(ShapeFunctionT T)
  {
    if( ShapeFunctionT::shape      == m_region.elements_type().getShape() &&
        ShapeFunctionT::order      == m_region.elements_type().getOrder() &&
        ShapeFunctionT::dimensions == m_region.elements_type().getDimensionality())
    {
      found = true;
      Gauss<ShapeFunctionT, ShapeFunctionT, IntegrationOrder>::integrateRegion(m_region, m_functor, m_result);
    }
  }

  /// True if a matching integrator was found
  bool& found;
private:
  const CRegion& m_region;
  FunctorT& m_functor;
  ResultT& m_result;
};

/// Gauss integration over a region
template<typename FunctorT, typename ResultT>
void gaussIntegrate(const CRegion& region, FunctorT& functor, ResultT& result)
{
  if(region.elements_count())
  {
    bool integrator_found = false;
    boost::mpl::for_each<LagrangeSF::SFTypes>(RegionIntegrator<FunctorT, ResultT>(region, functor, result, integrator_found));
    if(!integrator_found)
    {
      CFwarn << "no integrator found for region " << region.name() << CFendl;
    }
  }
}

/// Gauss integration over a mesh
/// The functor has all requirements for the integration functions of the Gauss class, and additionally needs to provide a
/// setRegion function. See IntegrationFunctorBase for a base class that provides a boilerplate implementation of non-functor-specific
/// functionality.
template<typename FunctorT, typename ResultT>
void gaussIntegrate(const CMesh& mesh, FunctorT& functor, ResultT& result)
{
  BOOST_FOREACH(const CRegion& region, recursive_range_typed<CRegion>(mesh))
  {
    CFdebug << "integrating region " << region.name() << " with " << region.elements_count() << " elements" << CFendl;
    functor.setRegion(region); // initialize region-specific functor data
    gaussIntegrate((region), functor, result);
  }
}

} // namespace Integrators
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_Integrators_Gauss_hpp */
