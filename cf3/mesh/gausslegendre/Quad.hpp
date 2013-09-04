// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_gausslegendre_Quad_hpp
#define cf3_mesh_gausslegendre_Quad_hpp

#include "mesh/QuadratureBase.hpp"
#include "mesh/gausslegendre/API.hpp"
#include "mesh/gausslegendre/Legendre.hpp"

namespace cf3 {
namespace mesh {
namespace gausslegendre {

////////////////////////////////////////////////////////////////////////////////

template <Uint P>
struct mesh_gausslegendre_API Quad_traits
{
  enum { nb_nodes       = P*P             };
  enum { dimensionality = 2               };
  enum { order          = P               };
  enum { shape          = GeoShape::QUAD  };
};

////////////////////////////////////////////////////////////////////////////////

/// @class Quad
/// @verbatim
/// Reference domain: <-1,1>, <-1,1>
/// @endverbatim
/// @see Quadrature for documentation on undocumented static functions
template <Uint P>
struct mesh_gausslegendre_API Quad : QuadratureBase< Quad_traits<P> >
{
public:
  typedef typename QuadratureBase< Quad_traits<P> >::LocalCoordsT  LocalCoordsT;
  typedef typename QuadratureBase< Quad_traits<P> >::WeightsT       WeightsT;

  static const LocalCoordsT& local_coordinates();
  static const WeightsT& weights();

private:

  // @brief Storage and conversion to static Eigen type
  struct GaussLegendreQuadrature
  {
    typename QuadratureBase< Quad_traits<P> >::LocalCoordsT roots;
    typename QuadratureBase< Quad_traits<P> >::WeightsT weights;

    static const GaussLegendreQuadrature& instance()
    {
      static GaussLegendreQuadrature inst;
      return inst;
    }

  private:

    GaussLegendreQuadrature()
    {
      std::pair< std::vector<Real>, std::vector<Real> > qdr = GaussLegendre(P);
      for (Uint i=0; i<P; ++i)
      {
        for (Uint j=0; j<P; ++j)
        {
          roots(j+P*i, KSI) = qdr.first[i];
          roots(j+P*i, ETA) = qdr.first[j];
          weights[j+P*i] = qdr.second[i] * qdr.second[j];
        }
      }
    }

  };

};

////////////////////////////////////////////////////////////////////////////////

template <Uint P>
const typename Quad<P>::WeightsT& Quad<P>::weights()
{
  static const WeightsT w = GaussLegendreQuadrature::instance().weights;
  return w;
}

////////////////////////////////////////////////////////////////////////////////

template <Uint P>
const typename Quad<P>::LocalCoordsT& Quad<P>::local_coordinates()
{
  static const LocalCoordsT loc_coord = GaussLegendreQuadrature::instance().roots;
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // gausslegendre
} // mesh
} // cf3

#endif // cf3_mesh_gausslegendre_Quad_hpp
