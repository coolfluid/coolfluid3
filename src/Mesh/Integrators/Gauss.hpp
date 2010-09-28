// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Integrators_Gauss_hpp
#define CF_Mesh_Integrators_Gauss_hpp

#include <boost/mpl/for_each.hpp>
#include <boost/foreach.hpp>
#include <boost/proto/proto.hpp>

#include "Common/AssertionManager.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"

#include "Mesh/Integrators/GaussImplementation.hpp"

#include "Mesh/SF/Types.hpp"



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
  static void integrateRegion(const CElements& region, FunctorT& functor, ResultT& result)
  {
    const Uint elem_begin = 0;
    const Uint elem_end = region.connectivity_table().array().size();
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
  RegionIntegrator(const CElements& region, FunctorT& functor, ResultT& result, bool& integratorFound)
    : found(integratorFound),
      m_region(region),
      m_functor(functor),
      m_result(result)
  {}

  template<typename ShapeFunctionT> void operator()(const ShapeFunctionT& T)
  {
    if( ShapeFunctionT::shape      == m_region.element_type().shape() &&
        ShapeFunctionT::order      == m_region.element_type().order() &&
        ShapeFunctionT::dimension == m_region.element_type().dimension())
    {
      found = true;
      Gauss<ShapeFunctionT, ShapeFunctionT, IntegrationOrder>::integrateRegion(m_region, m_functor, m_result);
    }
  }

  /// True if a matching integrator was found
  bool& found;
private:
  const CElements& m_region;
  FunctorT& m_functor;
  ResultT& m_result;
};

/// Gauss integration over a region
template<typename FunctorT, typename ResultT>
void gaussIntegrate(const CElements& region, FunctorT& functor, ResultT& result)
{
  bool integrator_found = false;
  boost::mpl::for_each<SF::VolumeTypes>(RegionIntegrator<FunctorT, ResultT>(region, functor, result, integrator_found));
  if(!integrator_found)
  {
    CFwarn << "no integrator found for region " << region.name() << CFendl;
  }
}

/// Gauss integration over a mesh
/// The functor has all requirements for the integration functions of the Gauss class, and additionally needs to provide a
/// setRegion function. See IntegrationFunctorBase for a base class that provides a boilerplate implementation of non-functor-specific
/// functionality.
template<typename FunctorT, typename ResultT>
void gaussIntegrate(const CMesh& mesh, FunctorT& functor, ResultT& result)
{
  BOOST_FOREACH(const CElements& region, recursive_range_typed<CElements>(mesh))
  {
    CFdebug << "integrating region " << region.name() << " with " << region.connectivity_table().array().size() << " elements" << CFendl;
    functor.setRegion(region); // initialize region-specific functor data
    gaussIntegrate((region), functor, result);
  }
}

////////////////////
// proto integrators
////////////////////

template<Uint Order, GeoShape::Type Shape>
struct GaussProto
{};

template<>
struct GaussProto<1, GeoShape::TRIAG>
{
  template<typename ExprT, typename ResultT, typename ContextT>
  static void integrate(ExprT& expr, ResultT& result, ContextT& context)
  {
    static const double mu = 0.3333333333333333333333333;
    static const double w = 0.5;
    context.mapped_coords.resize(2);
    context.mapped_coords[KSI] = mu;
    context.mapped_coords[ETA] = mu;
    result = w * boost::proto::eval(expr, context);
  }
};

template<>
struct GaussProto<1, GeoShape::TETRA>
{
  template<typename ExprT, typename ResultT, typename ContextT>
  static void integrate(ExprT& expr, ResultT& result, ContextT& context)
  {
    static const double mu = 0.25;
    static const double w = 0.1666666666666666666666667;
    context.mapped_coords.resize(3);
    context.mapped_coords[KSI] = mu;
    context.mapped_coords[ETA] = mu;
    context.mapped_coords[ZTA] = mu;
    result = w * boost::proto::eval(expr, context);
  }
};

template<>
struct GaussProto<1, GeoShape::LINE>
{
  template<typename ExprT, typename ResultT, typename ContextT>
  static void integrate(ExprT& expr, ResultT& result, ContextT& context)
  {
    static const double mu = 0.;
    static const double w = 2.;
    context.mapped_coords.resize(1);
    context.mapped_coords[KSI] = mu;
    result = w * boost::proto::eval(expr, context);
  }
};

template<Uint Order>
struct GaussProto<Order, GeoShape::LINE>
{
  template<typename ExprT, typename ResultT, typename ContextT>
  static void integrate(ExprT& expr, ResultT& result, ContextT& context)
  {
    // Init result to 0
    context.mapped_coords.resize(1, 0);
    result = boost::proto::eval(expr, context);
    result -= result;
    
    const static Uint npoints = Order/2;
    for(Uint i = 0; i != npoints; ++i)
    {
      const Real w = (GaussPoints<Order>::w[i]);
      context.mapped_coords[KSI] = GaussPoints<Order>::x[i];
      result += w*boost::proto::eval(expr, context);
      context.mapped_coords[KSI] = -GaussPoints<Order>::x[i];
      result += w*boost::proto::eval(expr, context);
    }
  }
};

template<>
struct GaussProto<1, GeoShape::QUAD>
{
  template<typename ExprT, typename ResultT, typename ContextT>
  static void integrate(ExprT& expr, ResultT& result, ContextT& context)
  {
    static const double mu = 0.;
    static const double w = 4.;
    context.mapped_coords.resize(2);
    context.mapped_coords[KSI] = mu;
    context.mapped_coords[ETA] = mu;
    result = w * boost::proto::eval(expr, context);
  }
};

template<Uint Order>
struct GaussProto<Order, GeoShape::QUAD>
{
  template<typename ExprT, typename ResultT, typename ContextT>
  static void integrate(ExprT& expr, ResultT& result, ContextT& context)
  {
    // Init result to 0
    context.mapped_coords.resize(2, 0);
    result = boost::proto::eval(expr, context);
    result -= result;
    
    const static Uint npoints = Order/2;
    for(Uint i = 0; i != npoints; ++i) {
      for(Uint j = 0; j != npoints; ++j) {
        const Real w = (GaussPoints<Order>::w[i] * GaussPoints<Order>::w[j]);
        context.mapped_coords[KSI] = GaussPoints<Order>::x[i];
        context.mapped_coords[ETA] = GaussPoints<Order>::x[j];
        result += w*boost::proto::eval(expr, context);
        context.mapped_coords[KSI] = -GaussPoints<Order>::x[i];
        context.mapped_coords[ETA] = GaussPoints<Order>::x[j];
        result += w*boost::proto::eval(expr, context);
        context.mapped_coords[KSI] = GaussPoints<Order>::x[i];
        context.mapped_coords[ETA] = -GaussPoints<Order>::x[j];
        result += w*boost::proto::eval(expr, context);
        context.mapped_coords[KSI] = -GaussPoints<Order>::x[i];
        context.mapped_coords[ETA] = -GaussPoints<Order>::x[j];
        result += w*boost::proto::eval(expr, context) ;
      }
    }
  }
};

template<>
struct GaussProto<1, GeoShape::HEXA>
{
  template<typename ExprT, typename ResultT, typename ContextT>
  static void integrate(ExprT& expr, ResultT& result, ContextT& context)
  {
    static const double mu = 0.;
    static const double w = 8.;
    context.mapped_coords.resize(3);
    context.mapped_coords[KSI] = mu;
    context.mapped_coords[ETA] = mu;
    context.mapped_coords[ETA] = mu;
    result = w * boost::proto::eval(expr, context);
  }
};

template<Uint Order>
struct GaussProto<Order, GeoShape::HEXA>
{
  template<typename ExprT, typename ResultT, typename ContextT>
  static void integrate(ExprT& expr, ResultT& result, ContextT& context)
  {
    // Init result to 0
    context.mapped_coords.resize(3, 0);
    result = boost::proto::eval(expr, context);
    result -= result;
    
    const static Uint npoints = Order/2;
    for(Uint i = 0; i != npoints; ++i) {
      for(Uint j = 0; j != npoints; ++j) {
        for(Uint k = 0; k != npoints; ++k) {
          const Real w = (GaussPoints<Order>::w[i] * GaussPoints<Order>::w[j] * GaussPoints<Order>::w[k]);
          
          context.mapped_coords[KSI] = GaussPoints<Order>::x[i];
          context.mapped_coords[ETA] = GaussPoints<Order>::x[j];
          context.mapped_coords[ETA] = GaussPoints<Order>::x[k];
          result += w*boost::proto::eval(expr, context);
          context.mapped_coords[KSI] = -GaussPoints<Order>::x[i];
          context.mapped_coords[ETA] = GaussPoints<Order>::x[j];
          context.mapped_coords[ETA] = GaussPoints<Order>::x[k];
          result += w*boost::proto::eval(expr, context);
          context.mapped_coords[KSI] = GaussPoints<Order>::x[i];
          context.mapped_coords[ETA] = -GaussPoints<Order>::x[j];
          context.mapped_coords[ETA] = GaussPoints<Order>::x[k];
          result += w*boost::proto::eval(expr, context);
          context.mapped_coords[KSI] = -GaussPoints<Order>::x[i];
          context.mapped_coords[ETA] = -GaussPoints<Order>::x[j];
          context.mapped_coords[ETA] = GaussPoints<Order>::x[k];
          result += w*boost::proto::eval(expr, context);
          
          context.mapped_coords[KSI] = GaussPoints<Order>::x[i];
          context.mapped_coords[ETA] = GaussPoints<Order>::x[j];
          context.mapped_coords[ETA] = -GaussPoints<Order>::x[k];
          result += w*boost::proto::eval(expr, context);
          context.mapped_coords[KSI] = -GaussPoints<Order>::x[i];
          context.mapped_coords[ETA] = GaussPoints<Order>::x[j];
          context.mapped_coords[ETA] = -GaussPoints<Order>::x[k];
          result += w*boost::proto::eval(expr, context);
          context.mapped_coords[KSI] = GaussPoints<Order>::x[i];
          context.mapped_coords[ETA] = -GaussPoints<Order>::x[j];
          context.mapped_coords[ETA] = -GaussPoints<Order>::x[k];
          result += w*boost::proto::eval(expr, context);
          context.mapped_coords[KSI] = -GaussPoints<Order>::x[i];
          context.mapped_coords[ETA] = -GaussPoints<Order>::x[j];
          context.mapped_coords[ETA] = -GaussPoints<Order>::x[k];
          result += w*boost::proto::eval(expr, context);
        }
      }
    }
  }
};

/// Integral of a proto expression. Context must define:
/// - SF: type of the shape function
/// - mapped_coords: RealVector with the mapped coordinates for the evaluation
template<Uint Order, typename ExprT, typename ResultT, typename ContextT>
void integrate(ExprT& expr, ResultT& result, ContextT& context)
{
  typedef typename ContextT::SF ShapeFunctionT;
  GaussProto<Order, ShapeFunctionT::shape>::integrate(expr, result, context);
}

} // namespace Integrators
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_Integrators_Gauss_hpp */
