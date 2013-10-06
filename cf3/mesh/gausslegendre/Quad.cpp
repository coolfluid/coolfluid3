// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/QuadratureT.hpp"
#include "mesh/gausslegendre/LibGaussLegendre.hpp"
#include "mesh/gausslegendre/Quad.hpp"

namespace cf3 {
namespace mesh {
namespace gausslegendre {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < QuadratureT< Quad<1> >, Quadrature, LibGaussLegendre >
   QuadP1_Builder(LibGaussLegendre::library_namespace()+"."+Quad<1>::type_name());

common::ComponentBuilder < QuadratureT< Quad<2> >, Quadrature, LibGaussLegendre >
   QuadP2_Builder(LibGaussLegendre::library_namespace()+"."+Quad<2>::type_name());

common::ComponentBuilder < QuadratureT< Quad<3> >, Quadrature, LibGaussLegendre >
   QuadP3_Builder(LibGaussLegendre::library_namespace()+"."+Quad<3>::type_name());

common::ComponentBuilder < QuadratureT< Quad<4> >, Quadrature, LibGaussLegendre >
   QuadP4_Builder(LibGaussLegendre::library_namespace()+"."+Quad<4>::type_name());

common::ComponentBuilder < QuadratureT< Quad<5> >, Quadrature, LibGaussLegendre >
   QuadP5_Builder(LibGaussLegendre::library_namespace()+"."+Quad<5>::type_name());

common::ComponentBuilder < QuadratureT< Quad<6> >, Quadrature, LibGaussLegendre >
   QuadP6_Builder(LibGaussLegendre::library_namespace()+"."+Quad<6>::type_name());

common::ComponentBuilder < QuadratureT< Quad<7> >, Quadrature, LibGaussLegendre >
   QuadP7_Builder(LibGaussLegendre::library_namespace()+"."+Quad<7>::type_name());

common::ComponentBuilder < QuadratureT< Quad<8> >, Quadrature, LibGaussLegendre >
   QuadP8_Builder(LibGaussLegendre::library_namespace()+"."+Quad<8>::type_name());

////////////////////////////////////////////////////////////////////////////////


} // gausslegendre
} // mesh
} // cf3
