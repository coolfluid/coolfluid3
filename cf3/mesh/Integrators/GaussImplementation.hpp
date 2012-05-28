// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Integrators_GaussImplementation_HH
#define cf3_mesh_Integrators_GaussImplementation_HH

#include <boost/assign/list_of.hpp>

#include "math/MatrixTypes.hpp"

#include "mesh/GeoShape.hpp"

namespace cf3 {
namespace mesh {
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
  static double const* x()
  {
    static const double x_arr[1] = {0.5773502691896257645091488};
    return x_arr;
  }

  static double const* w()
  {
    static const double w_arr[1] = {1.};
    return w_arr;
  }
};

template<>
struct GaussPoints<4>
{
  static double const* x()
  {
    static const double x_arr[2] = {0.3399810435848562648026658,0.8611363115940525752239465};
    return x_arr;
  }

  static double const* w()
  {
    static const double w_arr[2] = {0.6521451548625461426269361,0.3478548451374538573730639};
    return w_arr;
  }
};

template<>
struct GaussPoints<8>
{
  static double const* x()
  {
    static const double x_arr[4] = {0.1834346424956498049394761,0.5255324099163289858177390,0.7966664774136267395915539,0.9602898564975362316835609};
    return x_arr;
  }

  static double const* w()
  {
    static const double w_arr[4] = {0.3626837833783619829651504,0.3137066458778872873379622,0.2223810344533744705443560,0.1012285362903762591525314};
    return w_arr;
  }
};

template<>
struct GaussPoints<16>
{
  static double const* x()
  {
    static const double x_arr[8] = {0.0950125098376374401853193,0.2816035507792589132304605,0.4580167776572273863424194,0.6178762444026437484466718,0.7554044083550030338951012,0.8656312023878317438804679,0.9445750230732325760779884,0.9894009349916499325961542};
    return x_arr;
  }

  static double const* w()
  {
    static const double w_arr[8] = {0.1894506104550684962853967,0.1826034150449235888667637,0.1691565193950025381893121,0.1495959888165767320815017,0.1246289712555338720524763,0.0951585116824927848099251,0.0622535239386478928628438,0.0271524594117540948517806};
    return w_arr;
  }
};

template<>
struct GaussPoints<32>
{
  static double const* x()
  {
    static const double x_arr[16] = {0.0483076656877383162348126,0.1444719615827964934851864,0.2392873622521370745446032,0.3318686022821276497799168,0.4213512761306353453641194,0.5068999089322293900237475,0.5877157572407623290407455,0.6630442669302152009751152,0.7321821187402896803874267,0.7944837959679424069630973,0.8493676137325699701336930,0.8963211557660521239653072,0.9349060759377396891709191,0.9647622555875064307738119,0.9856115115452683354001750,0.9972638618494815635449811};
    return x_arr;
  }

  static double const* w()
  {
    static const double w_arr[16] = {0.0965400885147278005667648,0.0956387200792748594190820,0.0938443990808045656391802,0.0911738786957638847128686,0.0876520930044038111427715,0.0833119242269467552221991,0.0781938957870703064717409,0.0723457941088485062253994,0.0658222227763618468376501,0.0586840934785355471452836,0.0509980592623761761961632,0.0428358980222266806568786,0.0342738629130214331026877,0.0253920653092620594557526,0.0162743947309056706051706,0.0070186100094700966004071};
    return w_arr;
  }
};

/// Access to matrices with the mapped coords
template<Uint Order, GeoShape::Type Shape>
struct GaussMappedCoordsImpl;

template<>
struct GaussMappedCoordsImpl<1, GeoShape::LINE>
{
  static const Uint nb_points = 1;

  typedef Eigen::Matrix<Real, 1, 1> CoordsT;
  typedef Eigen::Matrix<Real, 1, 1> WeightsT;

  static CoordsT coords()
  {
    return CoordsT::Zero();
  }

  static WeightsT weights()
  {
    return 2. * WeightsT::Ones();
  }
};

template<>
struct GaussMappedCoordsImpl<1, GeoShape::QUAD>
{
  static const Uint nb_points = 1;

  typedef Eigen::Matrix<Real, 2, 1> CoordsT;
  typedef Eigen::Matrix<Real, 1, 1> WeightsT;

  static CoordsT coords()
  {
    return CoordsT::Zero();
  }

  static WeightsT weights()
  {
    return 4. * WeightsT::Ones();
  }
};

template<>
struct GaussMappedCoordsImpl<1, GeoShape::HEXA>
{
  static const Uint nb_points = 1;

  typedef Eigen::Matrix<Real, 3, 1> CoordsT;
  typedef Eigen::Matrix<Real, 1, 1> WeightsT;

  static CoordsT coords()
  {
    return CoordsT::Zero();
  }

  static WeightsT weights()
  {
    return 8. * WeightsT::Ones();
  }
};

template<>
struct GaussMappedCoordsImpl<1, GeoShape::TRIAG>
{
  static const Uint nb_points = 1;

  typedef Eigen::Matrix<Real, 2, 1> CoordsT;
  typedef Eigen::Matrix<Real, 1, 1> WeightsT;

  static CoordsT coords()
  {
    static const Real mu = 0.3333333333333333333333333;

    CoordsT result(mu, mu);
    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    result << 0.5;
    return result;
  }
};


template<>
struct GaussMappedCoordsImpl<2, GeoShape::TRIAG>
{
  static const Uint nb_points = 3;

  typedef Eigen::Matrix<Real, 2, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    CoordsT result;
    result.resize(DIM_2D, nb_points);
    result(KSI,0) = 0.5;
    result(ETA,0) = 0.0;
    result(KSI,1) = 0.5;
    result(ETA,1) = 0.5;
    result(KSI,2) = 0.0;
    result(ETA,2) = 0.5;

    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    result.resize(3);
    result(0) = 1.0/6.0;
    result(1) = 1.0/6.0;
    result(2) = 1.0/6.0;
    return result;
  }
};

template<>
struct GaussMappedCoordsImpl<3, GeoShape::TRIAG>
{
  static const Uint nb_points = 4;

  typedef Eigen::Matrix<Real, 2, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    CoordsT result;
    result.resize(DIM_2D, nb_points);
    result(KSI,0) = 1./3.;
    result(ETA,0) = 1./3.;
    result(KSI,1) = 0.2;
    result(ETA,1) = 0.2;
    result(KSI,2) = 0.6;
    result(ETA,2) = 0.2;
    result(KSI,3) = 0.2;
    result(ETA,3) = 0.6;

    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    result.resize(4);
    result(0) = -27./96.;
    result(1) =  25./96.;
    result(2) =  25./96.;
    result(3) =  25./96.;
    return result;
  }
};


template<>
struct GaussMappedCoordsImpl<4, GeoShape::TRIAG>
{
  static const Uint nb_points = 6;

  typedef Eigen::Matrix<Real, 2, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    CoordsT result;
    result.resize(DIM_2D, nb_points);
    result(KSI,0) = 0.091576213509771;
    result(ETA,0) = 0.091576213509771;
    result(KSI,1) = 0.816847572980459;
    result(ETA,1) = 0.091576213509771;
    result(KSI,2) = 0.091576213509771;
    result(ETA,2) = 0.816847572980459;
    result(KSI,3) = 0.445948490915965;
    result(ETA,3) = 0.445948490915965;
    result(KSI,4) = 0.108103018168070;
    result(ETA,4) = 0.445948490915965;
    result(KSI,5) = 0.445948490915965;
    result(ETA,5) = 0.108103018168070;

    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    result.resize(6);
    result(0) = 0.109951743655322/2.;
    result(1) = 0.109951743655322/2.;
    result(2) = 0.109951743655322/2.;
    result(3) = 0.223381589678011/2.;
    result(4) = 0.223381589678011/2.;
    result(5) = 0.223381589678011/2.;

    return result;
  }
};


template<>
struct GaussMappedCoordsImpl<5, GeoShape::TRIAG>
{
  static const Uint nb_points = 12;

  typedef Eigen::Matrix<Real, 2, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    CoordsT result;
    result.resize(DIM_2D, nb_points);
    result(KSI,0)  = 0.063089014491502;
    result(ETA,0)  = 0.063089014491502;
    result(KSI,1)  = 0.873821971016996;
    result(ETA,1)  = 0.063089014491502;
    result(KSI,2)  = 0.063089014491502;
    result(ETA,2)  = 0.873821971016996;
    result(KSI,3)  = 0.249286745170910;
    result(ETA,3)  = 0.501426509658179;
    result(KSI,4)  = 0.501426509658179;
    result(ETA,4)  = 0.249286745170910;
    result(KSI,5)  = 0.249286745170910;
    result(ETA,5)  = 0.249286745170910;
    result(KSI,6)  = 0.310352451033785;
    result(ETA,6)  = 0.053145049844816;
    result(KSI,7)  = 0.053145049844816;
    result(ETA,7)  = 0.310352451033785;
    result(KSI,8)  = 0.636502499121399;
    result(ETA,8)  = 0.053145049844816;
    result(KSI,9)  = 0.636502499121399;
    result(ETA,9)  = 0.310352451033785;
    result(KSI,10) = 0.053145049844816;
    result(ETA,10) = 0.636502499121399;
    result(KSI,11) = 0.310352451033785;
    result(ETA,11) = 0.636502499121399;

    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    result.resize(12);
    result(0)  = 0.050844906370207/2.;
    result(1)  = 0.050844906370207/2.;
    result(2)  = 0.050844906370207/2.;
    result(3)  = 0.116786275726379/2.;
    result(4)  = 0.116786275726379/2.;
    result(5)  = 0.116786275726379/2.;
    result(6)  = 0.082851075618374/2.;
    result(7)  = 0.082851075618374/2.;
    result(8)  = 0.082851075618374/2.;
    result(9)  = 0.082851075618374/2.;
    result(10) = 0.082851075618374/2.;
    result(11) = 0.082851075618374/2.;

    return result;
  }
};






template<>
struct GaussMappedCoordsImpl<1, GeoShape::TETRA>
{
  static const Uint nb_points = 1;

  typedef Eigen::Matrix<Real, 3, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    static const double mu = 0.25;

    CoordsT result;
    result << mu, mu, mu;
    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    result << 0.1666666666666666666666667;
    return result;
  }
};

template<>
struct GaussMappedCoordsImpl<2, GeoShape::TETRA>
{
  static const Uint nb_points = 4;

  typedef Eigen::Matrix<Real, 3, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    static const double mu = 0.13819660112501051517954131656;

    CoordsT result;
    result(KSI, 0) = mu;
    result(ETA, 0) = mu;
    result(ZTA, 0) = mu;
    result(KSI, 1) = 1.-3.*mu;
    result(ETA, 1) = mu;
    result(ZTA, 1) = mu;
    result(KSI, 2) = mu;
    result(ETA, 2) = 1.-3.*mu;
    result(ZTA, 2) = mu;
    result(KSI, 3) = mu;
    result(ETA, 3) = mu;
    result(ZTA, 3) = 1.-3.*mu;
    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    result.setConstant(0.04166666666666666666666666666666666667);
    return result;
  }
};


/// Trapezium rule integration. Uses the end points of the line.
template<>
struct GaussMappedCoordsImpl<777, GeoShape::LINE>
{
  static const Uint nb_points = 2;

  typedef Eigen::Matrix<Real, 1, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    CoordsT result;
    result.resize(DIM_1D, nb_points);
    result(KSI,0)  = -1.0;
    result(KSI,1)  =  1.0;

    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    result.resize(nb_points);
    result(0)  = 1.0;
    result(1)  = 1.0;
    return result;
  }
};

template<Uint Order>
struct GaussMappedCoordsImpl<Order, GeoShape::LINE>
{
  static const Uint nb_points = Order;

  typedef Eigen::Matrix<Real, 1, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    CoordsT result;
    const static Uint npoints = Order/2;
    Uint n = 0;
    for(Uint i = 0; i != npoints; ++i)
    {
      result.col(n++) << GaussPoints<Order>::x()[i];
      result.col(n++) << -GaussPoints<Order>::x()[i];
    }

    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    const static Uint npoints = Order/2;
    Uint n = 0;
    for(Uint i = 0; i != npoints; ++i)
    {
      const Real w = (GaussPoints<Order>::w()[i]);
      result.col(n++) << w;
      result.col(n++) << w;
    }

    return result;
  }
};

template<Uint Order>
struct GaussMappedCoordsImpl<Order, GeoShape::QUAD>
{
  static const Uint nb_points = Order*Order;

  typedef Eigen::Matrix<Real, 2, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    CoordsT result;
    const static Uint npoints = Order/2;
    Uint n = 0;
    for(Uint i = 0; i != npoints; ++i)
    {
      for(Uint j = 0; j != npoints; ++j)
      {
        result.col(n++) <<  GaussPoints<Order>::x()[i],  GaussPoints<Order>::x()[j];
        result.col(n++) << -GaussPoints<Order>::x()[i],  GaussPoints<Order>::x()[j];
        result.col(n++) <<  GaussPoints<Order>::x()[i], -GaussPoints<Order>::x()[j];
        result.col(n++) << -GaussPoints<Order>::x()[i], -GaussPoints<Order>::x()[j];
      }
    }

    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    const static Uint npoints = Order/2;
    Uint n = 0;
    for(Uint i = 0; i != npoints; ++i)
    {
      for(Uint j = 0; j != npoints; ++j)
      {
        const Real w = GaussPoints<Order>::w()[i] * GaussPoints<Order>::w()[j];
        result.col(n++) << w;
        result.col(n++) << w;
        result.col(n++) << w;
        result.col(n++) << w;
      }
    }

    return result;
  }
};

template<Uint Order>
struct GaussMappedCoordsImpl<Order, GeoShape::HEXA>
{
  static const Uint nb_points = Order*Order*Order;

  typedef Eigen::Matrix<Real, 3, nb_points> CoordsT;
  typedef Eigen::Matrix<Real, 1, nb_points> WeightsT;

  static CoordsT coords()
  {
    CoordsT result;
    const static Uint npoints = Order/2;
    Uint n = 0;
    for(Uint i = 0; i != npoints; ++i)
    {
      for(Uint j = 0; j != npoints; ++j)
      {
        for(Uint k = 0; k != npoints; ++k)
        {
          result.col(n++) <<  GaussPoints<Order>::x()[i],  GaussPoints<Order>::x()[j],  GaussPoints<Order>::x()[k];
          result.col(n++) << -GaussPoints<Order>::x()[i],  GaussPoints<Order>::x()[j],  GaussPoints<Order>::x()[k];
          result.col(n++) <<  GaussPoints<Order>::x()[i], -GaussPoints<Order>::x()[j],  GaussPoints<Order>::x()[k];
          result.col(n++) <<  GaussPoints<Order>::x()[i],  GaussPoints<Order>::x()[j], -GaussPoints<Order>::x()[k];
          result.col(n++) <<  GaussPoints<Order>::x()[i], -GaussPoints<Order>::x()[j], -GaussPoints<Order>::x()[k];
          result.col(n++) << -GaussPoints<Order>::x()[i],  GaussPoints<Order>::x()[j], -GaussPoints<Order>::x()[k];
          result.col(n++) << -GaussPoints<Order>::x()[i], -GaussPoints<Order>::x()[j],  GaussPoints<Order>::x()[k];
          result.col(n++) << -GaussPoints<Order>::x()[i], -GaussPoints<Order>::x()[j], -GaussPoints<Order>::x()[k];
        }
      }
    }

    return result;
  }

  static WeightsT weights()
  {
    WeightsT result;
    const static Uint npoints = Order/2;
    Uint n = 0;
    for(Uint i = 0; i != npoints; ++i)
    {
      for(Uint j = 0; j != npoints; ++j)
      {
        for(Uint k = 0; k != npoints; ++k)
        {
          const Real w = GaussPoints<Order>::w()[i] * GaussPoints<Order>::w()[j] * GaussPoints<Order>::w()[k];
          result.col(n++) << w;
          result.col(n++) << w;
          result.col(n++) << w;
          result.col(n++) << w;
          result.col(n++) << w;
          result.col(n++) << w;
          result.col(n++) << w;
          result.col(n++) << w;
        }
      }
    }

    return result;
  }
};

/// Stores pre-computed mapped coords and weights for all gauss point locations
template<Uint Order, GeoShape::Type Shape>
struct GaussMappedCoords
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  static const Uint nb_points = GaussMappedCoordsImpl<Order, Shape>::nb_points;

  typedef typename GaussMappedCoordsImpl<Order, Shape>::CoordsT CoordsT;
  typedef typename GaussMappedCoordsImpl<Order, Shape>::WeightsT WeightsT;

  const CoordsT coords;
  const WeightsT weights;

  static const GaussMappedCoords<Order, Shape>& instance()
  {
    static GaussMappedCoords<Order, Shape> data;
    return data;
  }

private:

  GaussMappedCoords() :
    coords(GaussMappedCoordsImpl<Order, Shape>::coords()),
    weights(GaussMappedCoordsImpl<Order, Shape>::weights()) {}

};

template<Uint Order, GeoShape::Type Shape>
struct GaussIntegrator;

template<>
struct GaussIntegrator<1, GeoShape::TRIAG>
{
  template<typename FunctorT, typename MappedCoordsT, typename ResultT>
  static void integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
  {
    static const double mu = 0.3333333333333333333333333;
    static const double w = 0.5;
    mapped_coords.resize(2);
    mapped_coords[KSI] = mu;
    mapped_coords[ETA] = mu;
    result = w * functor();
  }
};

template<>
struct GaussIntegrator<2, GeoShape::TRIAG>
{
  template<typename FunctorT, typename MappedCoordsT, typename ResultT>
  static void integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
  {
    static const double w = 1.0/6.0;
    mapped_coords.resize(2,0);
    mapped_coords[KSI] = 0.5;
    mapped_coords[ETA] = 0.0;
    result = w * functor();
    mapped_coords[KSI] = 0.5;
    mapped_coords[ETA] = 0.5;
    result += w * functor();
    mapped_coords[KSI] = 0.0;
    mapped_coords[ETA] = 0.5;
    result += w * functor();
  }
};

template<>
struct GaussIntegrator<1, GeoShape::TETRA>
{
  template<typename FunctorT, typename MappedCoordsT, typename ResultT>
  static void integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
  {
    static const double mu = 0.25;
    static const double w = 0.1666666666666666666666667;
    mapped_coords.resize(3);
    mapped_coords[KSI] = mu;
    mapped_coords[ETA] = mu;
    mapped_coords[ZTA] = mu;
    result = w * functor();
  }
};

template<>
struct GaussIntegrator<1, GeoShape::LINE>
{
  template<typename FunctorT, typename MappedCoordsT, typename ResultT>
  static void integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
  {
    static const double mu = 0.;
    static const double w = 2.;
    mapped_coords[KSI] = mu;
    result = w * functor();
  }
};

template<Uint Order>
struct GaussIntegrator<Order, GeoShape::LINE>
{
  template<typename FunctorT, typename MappedCoordsT, typename ResultT>
  static void integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
  {
    // Init result to 0
    mapped_coords.resize(1, 0);
    result = functor();
    result -= result;

    const static Uint npoints = Order/2;
    for(Uint i = 0; i != npoints; ++i)
    {
      const Real w = (GaussPoints<Order>::w()[i]);
      mapped_coords[KSI] = GaussPoints<Order>::x()[i];
      result += w*functor();
      mapped_coords[KSI] = -GaussPoints<Order>::x()[i];
      result += w*functor();
    }
  }
};

template<>
struct GaussIntegrator<1, GeoShape::QUAD>
{
  template<typename FunctorT, typename MappedCoordsT, typename ResultT>
  static void integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
  {
    static const double mu = 0.;
    static const double w = 4.;
    mapped_coords.resize(2);
    mapped_coords[KSI] = mu;
    mapped_coords[ETA] = mu;
    result = w * functor();
  }
};

template<Uint Order>
struct GaussIntegrator<Order, GeoShape::QUAD>
{
  template<typename FunctorT, typename MappedCoordsT, typename ResultT>
  static void integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
  {
    // Init result to 0
    mapped_coords.resize(2, 0);
    result = functor();
    result -= result;

    const static Uint npoints = Order/2;
    for(Uint i = 0; i != npoints; ++i) {
      for(Uint j = 0; j != npoints; ++j) {
        const Real w = (GaussPoints<Order>::w()[i] * GaussPoints<Order>::w()[j]);
        mapped_coords[KSI] = GaussPoints<Order>::x()[i];
        mapped_coords[ETA] = GaussPoints<Order>::x()[j];
        result += w*functor();
        mapped_coords[KSI] = -GaussPoints<Order>::x()[i];
        mapped_coords[ETA] = GaussPoints<Order>::x()[j];
        result += w*functor();
        mapped_coords[KSI] = GaussPoints<Order>::x()[i];
        mapped_coords[ETA] = -GaussPoints<Order>::x()[j];
        result += w*functor();
        mapped_coords[KSI] = -GaussPoints<Order>::x()[i];
        mapped_coords[ETA] = -GaussPoints<Order>::x()[j];
        result += w*functor() ;
      }
    }
  }
};

template<>
struct GaussIntegrator<1, GeoShape::HEXA>
{
  template<typename FunctorT, typename MappedCoordsT, typename ResultT>
  static void integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
  {
    static const double mu = 0.;
    static const double w = 8.;
    mapped_coords.resize(3);
    mapped_coords[KSI] = mu;
    mapped_coords[ETA] = mu;
    mapped_coords[ETA] = mu;
    result = w * functor();
  }
};

template<Uint Order>
struct GaussIntegrator<Order, GeoShape::HEXA>
{
  template<typename FunctorT, typename MappedCoordsT, typename ResultT>
  static void integrate(const FunctorT& functor, MappedCoordsT& mapped_coords, ResultT& result)
  {
    // Init result to 0
    mapped_coords.resize(3, 0);
    result = functor();
    result -= result;

    const static Uint npoints = Order/2;
    for(Uint i = 0; i != npoints; ++i) {
      for(Uint j = 0; j != npoints; ++j) {
        for(Uint k = 0; k != npoints; ++k) {
          const Real w = (GaussPoints<Order>::w()[i] * GaussPoints<Order>::w()[j] * GaussPoints<Order>::w()[k]);

          mapped_coords[KSI] = GaussPoints<Order>::x()[i];
          mapped_coords[ETA] = GaussPoints<Order>::x()[j];
          mapped_coords[ETA] = GaussPoints<Order>::x()[k];
          result += w*functor();
          mapped_coords[KSI] = -GaussPoints<Order>::x()[i];
          mapped_coords[ETA] = GaussPoints<Order>::x()[j];
          mapped_coords[ETA] = GaussPoints<Order>::x()[k];
          result += w*functor();
          mapped_coords[KSI] = GaussPoints<Order>::x()[i];
          mapped_coords[ETA] = -GaussPoints<Order>::x()[j];
          mapped_coords[ETA] = GaussPoints<Order>::x()[k];
          result += w*functor();
          mapped_coords[KSI] = -GaussPoints<Order>::x()[i];
          mapped_coords[ETA] = -GaussPoints<Order>::x()[j];
          mapped_coords[ETA] = GaussPoints<Order>::x()[k];
          result += w*functor();

          mapped_coords[KSI] = GaussPoints<Order>::x()[i];
          mapped_coords[ETA] = GaussPoints<Order>::x()[j];
          mapped_coords[ETA] = -GaussPoints<Order>::x()[k];
          result += w*functor();
          mapped_coords[KSI] = -GaussPoints<Order>::x()[i];
          mapped_coords[ETA] = GaussPoints<Order>::x()[j];
          mapped_coords[ETA] = -GaussPoints<Order>::x()[k];
          result += w*functor();
          mapped_coords[KSI] = GaussPoints<Order>::x()[i];
          mapped_coords[ETA] = -GaussPoints<Order>::x()[j];
          mapped_coords[ETA] = -GaussPoints<Order>::x()[k];
          result += w*functor();
          mapped_coords[KSI] = -GaussPoints<Order>::x()[i];
          mapped_coords[ETA] = -GaussPoints<Order>::x()[j];
          mapped_coords[ETA] = -GaussPoints<Order>::x()[k];
          result += w*functor();
        }
      }
    }
  }
};


} // Gauss
} // mesh
} // cf3

#endif /* CF3_Mesh_Integrators_Gaussimplementation_HH */
