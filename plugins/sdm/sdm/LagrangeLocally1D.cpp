// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP0/Point.hpp"
#include "sdm/LibSDM.hpp"
#include "sdm/LagrangeLocally1D.hpp"
#include "common/Builder.hpp"

using namespace cf3::common;

namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder<Point<0>,mesh::ShapeFunction,LibSDM>
PointP0_builder(LibSDM::library_namespace()+".P0.Point");

ComponentBuilder<Point<1>,mesh::ShapeFunction,LibSDM>
PointP1_builder(LibSDM::library_namespace()+".P1.Point");

ComponentBuilder<Point<2>,mesh::ShapeFunction,LibSDM>
PointP2_builder(LibSDM::library_namespace()+".P2.Point");

ComponentBuilder<Point<3>,mesh::ShapeFunction,LibSDM>
PointP3_builder(LibSDM::library_namespace()+".P3.Point");

ComponentBuilder<Point<4>,mesh::ShapeFunction,LibSDM>
PointP4_builder(LibSDM::library_namespace()+".P4.Point");

ComponentBuilder<Point<5>,mesh::ShapeFunction,LibSDM>
PointP5_builder(LibSDM::library_namespace()+".P5.Point");

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder<LineLagrange1D<0>,mesh::ShapeFunction,LibSDM>
LineP0_builder(LibSDM::library_namespace()+".P0.Line");

ComponentBuilder<LineLagrange1D<1>,mesh::ShapeFunction,LibSDM>
LineP1_builder(LibSDM::library_namespace()+".P1.Line");

ComponentBuilder<LineLagrange1D<2>,mesh::ShapeFunction,LibSDM>
LineP2_builder(LibSDM::library_namespace()+".P2.Line");

ComponentBuilder<LineLagrange1D<3>,mesh::ShapeFunction,LibSDM>
LineP3_builder(LibSDM::library_namespace()+".P3.Line");

ComponentBuilder<LineLagrange1D<4>,mesh::ShapeFunction,LibSDM>
LineP4_builder(LibSDM::library_namespace()+".P4.Line");

ComponentBuilder<LineLagrange1D<5>,mesh::ShapeFunction,LibSDM>
LineP5_builder(LibSDM::library_namespace()+".P5.Line");

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder<QuadLagrange1D<0>,mesh::ShapeFunction,LibSDM>
QuadP0_builder(LibSDM::library_namespace()+".P0.Quad");

ComponentBuilder<QuadLagrange1D<1>,mesh::ShapeFunction,LibSDM>
QuadP1_builder(LibSDM::library_namespace()+".P1.Quad");

ComponentBuilder<QuadLagrange1D<2>,mesh::ShapeFunction,LibSDM>
QuadP2_builder(LibSDM::library_namespace()+".P2.Quad");

ComponentBuilder<QuadLagrange1D<3>,mesh::ShapeFunction,LibSDM>
QuadP3_builder(LibSDM::library_namespace()+".P3.Quad");

ComponentBuilder<QuadLagrange1D<4>,mesh::ShapeFunction,LibSDM>
QuadP4_builder(LibSDM::library_namespace()+".P4.Quad");

ComponentBuilder<QuadLagrange1D<5>,mesh::ShapeFunction,LibSDM>
QuadP5_builder(LibSDM::library_namespace()+".P5.Quad");

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder<HexaLagrange1D<0>,mesh::ShapeFunction,LibSDM>
HexaP0_builder(LibSDM::library_namespace()+".P0.Hexa");

ComponentBuilder<HexaLagrange1D<1>,mesh::ShapeFunction,LibSDM>
HexaP1_builder(LibSDM::library_namespace()+".P1.Hexa");

ComponentBuilder<HexaLagrange1D<2>,mesh::ShapeFunction,LibSDM>
HexaP2_builder(LibSDM::library_namespace()+".P2.Hexa");

ComponentBuilder<HexaLagrange1D<3>,mesh::ShapeFunction,LibSDM>
HexaP3_builder(LibSDM::library_namespace()+".P3.Hexa");

ComponentBuilder<HexaLagrange1D<4>,mesh::ShapeFunction,LibSDM>
HexaP4_builder(LibSDM::library_namespace()+".P4.Hexa");

ComponentBuilder<HexaLagrange1D<5>,mesh::ShapeFunction,LibSDM>
HexaP5_builder(LibSDM::library_namespace()+".P5.Hexa");

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

