// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/std/vector.hpp>
#include "math/Consts.hpp"
#include "mesh/gausslegendre/Legendre.hpp"

using namespace boost::assign;

namespace cf3 {
namespace mesh {
namespace gausslegendre {

////////////////////////////////////////////////////////////////////////////////

Real Legendre(const Uint n, const Real& x)
{
  if (n==0)
    return 1.0;
  else if (n==1)
    return x;
  else
    return ((2.0*static_cast<Real>(n)-1.0)*x*Legendre(n-1,x)-(n-1)*Legendre(n-2,x))/static_cast<Real>(n);
 }

Real DLegendre(const Uint n, const Real& x)
{
  if (n==0)
    return 0.;
  else if (n==1)
    return 1.;
  else
    return (static_cast<Real>(n)/(x*x-1.0))*(x*Legendre(n,x)-Legendre(n-1,x));
}

std::vector<Real> GaussLegendreRoots(const Uint polyorder, const Real& tolerance)
{
  using namespace std;
  std::vector<Real> roots;
  switch (polyorder)
  {
    case 1:
      roots += 0.;
      break;
    case 2:
      roots += -1./sqrt(3.), 1./sqrt(3.);
      break;
    case 3:
      roots += -sqrt(3./5.), 0., sqrt(3./5.);
      break;
    case 4:
      roots += -sqrt( (3.+2.*sqrt(6./5.))/7. ), 
               -sqrt( (3.-2.*sqrt(6./5.))/7. ),
                sqrt( (3.-2.*sqrt(6./5.))/7. ),
                sqrt( (3.+2.*sqrt(6./5.))/7. );
      break;
    case 5:
      roots += -1./3.*sqrt( 5.+2.*sqrt(10./7.) ), 
               -1./3.*sqrt( 5.-2.*sqrt(10./7.) ),
                0.,
                1./3.*sqrt( 5.-2.*sqrt(10./7.) ),
                1./3.*sqrt( 5.+2.*sqrt(10./7.) );
      break;
    default:
    {
      roots.resize(polyorder);
      for (Uint i=1; i<=polyorder; ++i)
      {
        Real x=std::cos(math::Consts::pi()*(i-0.25)/(polyorder+0.5));
        Real error=10.*tolerance;
        Uint iters=0;
        while (error>tolerance && iters<1000)
        {
          const Real dx = -Legendre(polyorder,x)/DLegendre(polyorder,x);
          x += dx;
          ++iters;
          error=std::abs(dx);
        }
        roots[i-1] = x;
      }
    }
  }
  return roots;
}

std::vector<Real> GaussLegendreWeights(const std::vector<Real>& roots)
{
  const Uint polyorder = roots.size();
  std::vector<Real> weights;
  using namespace std;
  switch (polyorder)
  {
    case 1:
      weights += 2.;
      break;
    case 2:
      weights += 1., 1.;
      break;
    case 3:
      weights += 5./9., 8./9., 5./9.;
      break;
    case 4:
      weights += (18.-sqrt(30.))/36., 
                 (18.+sqrt(30.))/36.,
                 (18.+sqrt(30.))/36.,
                 (18.-sqrt(30.))/36.;
      break;
    case 5:
      weights += (322.-13.*sqrt(70.))/900., 
                 (322.+13.*sqrt(70.))/900.,
                 128./225.,
                 (322.+13.*sqrt(70.))/900.,
                 (322.-13.*sqrt(70.))/900.;
      break;
    default:
    {
      weights.resize(polyorder);
      for (Uint i=0; i<polyorder; ++i)
      {
        const Real& r = roots[i];
        const Real& dl = DLegendre(polyorder,r);
        weights[i] = 2.0 / ( (1.0-r*r)*(dl*dl) );
      }
    }
  }
  return weights;
}

std::pair< std::vector<Real>, std::vector<Real> > GaussLegendre(const Uint polyorder)
{
  std::vector<Real> roots   = GaussLegendreRoots(polyorder);
  std::vector<Real> weights = GaussLegendreWeights(roots);
  return std::make_pair(roots,weights);
}

////////////////////////////////////////////////////////////////////////////////

} // gausslegendre
} // mesh
} // cf3
