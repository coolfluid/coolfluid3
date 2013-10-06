// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/QuadratureT.hpp"
#include "mesh/gausslegendre/LibGaussLegendre.hpp"
#include "mesh/gausslegendre/Line.hpp"

namespace cf3 {
namespace mesh {
namespace gausslegendre {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < QuadratureT< Line<1> >, Quadrature, LibGaussLegendre >
   LineP1_Builder(LibGaussLegendre::library_namespace()+"."+Line<1>::type_name());

common::ComponentBuilder < QuadratureT< Line<2> >, Quadrature, LibGaussLegendre >
   LineP2_Builder(LibGaussLegendre::library_namespace()+"."+Line<2>::type_name());

common::ComponentBuilder < QuadratureT< Line<3> >, Quadrature, LibGaussLegendre >
   LineP3_Builder(LibGaussLegendre::library_namespace()+"."+Line<3>::type_name());

common::ComponentBuilder < QuadratureT< Line<4> >, Quadrature, LibGaussLegendre >
   LineP4_Builder(LibGaussLegendre::library_namespace()+"."+Line<4>::type_name());

common::ComponentBuilder < QuadratureT< Line<5> >, Quadrature, LibGaussLegendre >
   LineP5_Builder(LibGaussLegendre::library_namespace()+"."+Line<5>::type_name());

common::ComponentBuilder < QuadratureT< Line<6> >, Quadrature, LibGaussLegendre >
   LineP6_Builder(LibGaussLegendre::library_namespace()+"."+Line<6>::type_name());

common::ComponentBuilder < QuadratureT< Line<7> >, Quadrature, LibGaussLegendre >
   LineP7_Builder(LibGaussLegendre::library_namespace()+"."+Line<7>::type_name());

common::ComponentBuilder < QuadratureT< Line<8> >, Quadrature, LibGaussLegendre >
   LineP8_Builder(LibGaussLegendre::library_namespace()+"."+Line<8>::type_name());

////////////////////////////////////////////////////////////////////////////////


} // gausslegendre
} // mesh
} // cf3
