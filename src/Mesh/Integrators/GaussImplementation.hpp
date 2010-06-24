#ifndef CF_Mesh_Integrators_GaussImplementation_HH
#define CF_Mesh_Integrators_Gauss_HH

#include <boost/assign/list_of.hpp>

#include "Common/AssertionManager.hpp"
#include "Mesh/GeoShape.hpp"
#include "Math/RealVector.hpp"

namespace CF {
namespace Mesh {
namespace Integrators {

/// This file hides some implementation details of Gauss integration

/// Provide static precomputed access to Gauss points
template<Uint Order>
struct GaussPoints
{
};

template<>
struct GaussPoints<2>
{
  const static double x[1];
  const static double w[1];
};

const double GaussPoints<2>::x[1] = {0.5773502691896257645091488};
const double GaussPoints<2>::w[1] = {1.0000000000000000000000000};

template<>
struct GaussPoints<4>
{
  const static double x[2];
  const static double w[2];
};

const double GaussPoints<4>::x[2] = {0.3399810435848562648026658,0.8611363115940525752239465};
const double GaussPoints<4>::w[2] = {0.6521451548625461426269361,0.3478548451374538573730639};

template<>
struct GaussPoints<8>
{
  const static double x[4];
  const static double w[4];
};

const double GaussPoints<8>::x[4] = {0.1834346424956498049394761,0.5255324099163289858177390,0.7966664774136267395915539,0.9602898564975362316835609};
const double GaussPoints<8>::w[4] = {0.3626837833783619829651504,0.3137066458778872873379622,0.2223810344533744705443560,0.1012285362903762591525314};

template<>
struct GaussPoints<16>
{
  const static double x[8];
  const static double w[8];
};

const double GaussPoints<16>::x[8] = {0.0950125098376374401853193,0.2816035507792589132304605,0.4580167776572273863424194,0.6178762444026437484466718,0.7554044083550030338951012,0.8656312023878317438804679,0.9445750230732325760779884,0.9894009349916499325961542};
const double GaussPoints<16>::w[8] = {0.1894506104550684962853967,0.1826034150449235888667637,0.1691565193950025381893121,0.1495959888165767320815017,0.1246289712555338720524763,0.0951585116824927848099251,0.0622535239386478928628438,0.0271524594117540948517806};

template<>
struct GaussPoints<32>
{
  const static double x[16];
  const static double w[16];
};

const double GaussPoints<32>::x[16] = {0.0483076656877383162348126,0.1444719615827964934851864,0.2392873622521370745446032,0.3318686022821276497799168,0.4213512761306353453641194,0.5068999089322293900237475,0.5877157572407623290407455,0.6630442669302152009751152,0.7321821187402896803874267,0.7944837959679424069630973,0.8493676137325699701336930,0.8963211557660521239653072,0.9349060759377396891709191,0.9647622555875064307738119,0.9856115115452683354001750,0.9972638618494815635449811};
const double GaussPoints<32>::w[16] = {0.0965400885147278005667648,0.0956387200792748594190820,0.0938443990808045656391802,0.0911738786957638847128686,0.0876520930044038111427715,0.0833119242269467552221991,0.0781938957870703064717409,0.0723457941088485062253994,0.0658222227763618468376501,0.0586840934785355471452836,0.0509980592623761761961632,0.0428358980222266806568786,0.0342738629130214331026877,0.0253920653092620594557526,0.0162743947309056706051706,0.0070186100094700966004071};

/// Implementations of Gauss integration for different kinds of shape and orders
template<GeoShape::Type GeometryShape, GeoShape::Type SolutionShape, Uint Order>
class GaussImplementation
{
public:
  template<typename GeoShapeF, typename SolShapeF, typename FunctorT, typename ResultT>
  static void integrate(FunctorT& functor, ResultT& Result)
  {
    BOOST_STATIC_ASSERT(sizeof(ResultT) == 0); // Break compilation on non-specialized instantiation
  }
};

template<>
class GaussImplementation<GeoShape::TRIAG, GeoShape::TRIAG, 1>
{
public:
  template<typename GeoShapeF, typename SolShapeF, typename FunctorT, typename ResultT>
  static void integrate(FunctorT& functor, ResultT& Result)
  {
    static const double mu = 0.3333333333333333333333333;
    static const double w = 0.5;
    static const RealVector mapped_coords = boost::assign::list_of(mu)(mu);
    Result += w * functor.template valTimesDetJacobian<GeoShapeF, SolShapeF>(mapped_coords);
  }
};

template<>
class GaussImplementation<GeoShape::TETRA, GeoShape::TETRA, 1>
{
public:
  template<typename GeoShapeF, typename SolShapeF, typename FunctorT, typename ResultT>
  static void integrate(FunctorT& functor, ResultT& Result)
  {
    static const double mu = 0.25;
    static const double w = 0.1666666666666666666666667;
    static const RealVector mapped_coords = boost::assign::list_of(mu)(mu)(mu);
    Result += w * functor.template valTimesDetJacobian<GeoShapeF, SolShapeF>(mapped_coords);
  }
};

template<>
class GaussImplementation<GeoShape::QUAD, GeoShape::QUAD, 1>
{
public:
  template<typename GeoShapeF, typename SolShapeF, typename FunctorT, typename ResultT>
  static void integrate(FunctorT& functor, ResultT& Result)
  {
    static const double mu = 0.;
    static const double w = 4.;
    static const RealVector mapped_coords = boost::assign::list_of(mu)(mu);
    Result += w * functor.template valTimesDetJacobian<GeoShapeF, SolShapeF>(mapped_coords);
  }
};

template<Uint Order>
class GaussImplementation<GeoShape::QUAD, GeoShape::QUAD, Order>
{
public:
  template<typename GeoShapeF, typename SolShapeF, typename FunctorT, typename ResultT>
  static void integrate(FunctorT& functor, ResultT& result)
  {
    const static Uint npoints = Order/2;
    for(Uint i = 0; i != npoints; ++i) {
      for(Uint j = 0; j != npoints; ++j) {
        const RealVector a = boost::assign::list_of( GaussPoints<Order>::x[i])( GaussPoints<Order>::x[j]);
        const RealVector b = boost::assign::list_of(-GaussPoints<Order>::x[i])( GaussPoints<Order>::x[j]);
        const RealVector c = boost::assign::list_of( GaussPoints<Order>::x[i])(-GaussPoints<Order>::x[j]);
        const RealVector d = boost::assign::list_of(-GaussPoints<Order>::x[i])(-GaussPoints<Order>::x[j]);
        result += ((GaussPoints<Order>::w[i] * GaussPoints<Order>::w[j]) * (functor.template valTimesDetJacobian<GeoShapeF, SolShapeF>(a) + functor.template valTimesDetJacobian<GeoShapeF, SolShapeF>(b) + functor.template valTimesDetJacobian<GeoShapeF, SolShapeF>(c) + functor.template valTimesDetJacobian<GeoShapeF, SolShapeF>(d)));
      }
    }
  }
};

} // namespace Gauss
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_Integrators_Gauss_HH */
