// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/QuadratureT.hpp"
#include "mesh/gausslegendre/LibGaussLegendre.hpp"
#include "mesh/gausslegendre/Hexa.hpp"

namespace cf3 {
namespace mesh {
namespace gausslegendre {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < QuadratureT< Hexa<1> >, Quadrature, LibGaussLegendre >
   HexaP1_Builder(LibGaussLegendre::library_namespace()+"."+Hexa<1>::type_name());

common::ComponentBuilder < QuadratureT< Hexa<2> >, Quadrature, LibGaussLegendre >
   HexaP2_Builder(LibGaussLegendre::library_namespace()+"."+Hexa<2>::type_name());

common::ComponentBuilder < QuadratureT< Hexa<3> >, Quadrature, LibGaussLegendre >
   HexaP3_Builder(LibGaussLegendre::library_namespace()+"."+Hexa<3>::type_name());

common::ComponentBuilder < QuadratureT< Hexa<4> >, Quadrature, LibGaussLegendre >
   HexaP4_Builder(LibGaussLegendre::library_namespace()+"."+Hexa<4>::type_name());

common::ComponentBuilder < QuadratureT< Hexa<5> >, Quadrature, LibGaussLegendre >
   HexaP5_Builder(LibGaussLegendre::library_namespace()+"."+Hexa<5>::type_name());

common::ComponentBuilder < QuadratureT< Hexa<6> >, Quadrature, LibGaussLegendre >
   HexaP6_Builder(LibGaussLegendre::library_namespace()+"."+Hexa<6>::type_name());

common::ComponentBuilder < QuadratureT< Hexa<7> >, Quadrature, LibGaussLegendre >
   HexaP7_Builder(LibGaussLegendre::library_namespace()+"."+Hexa<7>::type_name());

common::ComponentBuilder < QuadratureT< Hexa<8> >, Quadrature, LibGaussLegendre >
   HexaP8_Builder(LibGaussLegendre::library_namespace()+"."+Hexa<8>::type_name());

////////////////////////////////////////////////////////////////////////////////


} // gausslegendre
} // mesh
} // cf3
