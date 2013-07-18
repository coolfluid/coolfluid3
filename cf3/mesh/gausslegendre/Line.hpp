// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_gausslegendre_Line_hpp
#define cf3_mesh_gausslegendre_Line_hpp

#include "mesh/QuadratureBase.hpp"
#include "mesh/gausslegendre/API.hpp"
#include "mesh/gausslegendre/Legendre.hpp"

namespace cf3 {
namespace mesh {
namespace gausslegendre {

////////////////////////////////////////////////////////////////////////////////

template <Uint P>
struct mesh_gausslegendre_API Line_traits
{
  enum { nb_nodes       = P               };
  enum { dimensionality = 1               };
  enum { order          = P               };
  enum { shape          = GeoShape::LINE  };
};


// @brief Storage and conversion to static Eigen type
template <Uint P>
struct GaussLegendreQuadrature
{
  typename QuadratureBase< Line_traits<P> >::LocalCoordsT roots;
  typename QuadratureBase< Line_traits<P> >::WeightsT weights;

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
      roots[i] = qdr.first[i];
      weights[i] = qdr.second[i];
    }
  }

};

////////////////////////////////////////////////////////////////////////////////

/// @class Line
/// @verbatim
/// Reference domain: <-1,1>
/// @endverbatim
/// @see Quadrature for documentation on undocumented static functions
template <Uint P>
struct mesh_gausslegendre_API Line : QuadratureBase< Line_traits<P> >
{
public:
  typedef typename QuadratureBase< Line_traits<P> >::LocalCoordsT  LocalCoordsT;
  typedef typename QuadratureBase< Line_traits<P> >::WeightsT       WeightsT;

  static const LocalCoordsT& local_coordinates();
  static const WeightsT& weights();
};

////////////////////////////////////////////////////////////////////////////////

template <Uint P>
const typename Line<P>::WeightsT& Line<P>::weights()
{
  static const WeightsT w = GaussLegendreQuadrature<P>::instance().weights;
  return w;
}

////////////////////////////////////////////////////////////////////////////////

template <Uint P>
const typename Line<P>::LocalCoordsT& Line<P>::local_coordinates()
{
  static const LocalCoordsT loc_coord = GaussLegendreQuadrature<P>::instance().roots;
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // gausslegendre
} // mesh
} // cf3

#endif // cf3_mesh_gausslegendre_Line_hpp
