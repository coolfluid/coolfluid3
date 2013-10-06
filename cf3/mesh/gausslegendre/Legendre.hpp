// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_gausslegendre_Legendre_hpp
#define cf3_mesh_gausslegendre_Legendre_hpp

namespace cf3 {
namespace mesh {
namespace gausslegendre {

////////////////////////////////////////////////////////////////////////////////

// Legendre polynomials
Real Legendre(const Uint n, const Real& x);

// Derivative of the Legendre polynomials
Real DLegendre(const Uint n, const Real& x);

// Roots of the polynomial obtained using Newton-Raphson method
std::vector<Real> GaussLegendreRoots(const Uint polyorder, const Real& tolerance=1e-20);

// Weights forming the gauss quadrature
std::vector<Real> GaussLegendreWeights(const std::vector<Real>& roots);

// Roots and weights forming the gauss quadrature
std::pair< std::vector<Real>, std::vector<Real> > GaussLegendre(const Uint polyorder);

////////////////////////////////////////////////////////////////////////////////

} // gausslegendre
} // mesh
} // cf3

#endif // cf3_mesh_gausslegendre_Legendre_hpp
