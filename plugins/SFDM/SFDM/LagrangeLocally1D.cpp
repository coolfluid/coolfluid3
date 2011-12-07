// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "SFDM/LibSFDM.hpp"
#include "SFDM/LagrangeLocally1D.hpp"
#include "common/Builder.hpp"

using namespace cf3::common;

namespace cf3 {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder<QuadLagrange1D<0>,mesh::ShapeFunction,LibSFDM>
QuadP0_builder(LibSFDM::library_namespace()+".P0.Quad");

ComponentBuilder<QuadLagrange1D<1>,mesh::ShapeFunction,LibSFDM>
QuadP1_builder(LibSFDM::library_namespace()+".P1.Quad");

ComponentBuilder<QuadLagrange1D<2>,mesh::ShapeFunction,LibSFDM>
QuadP2_builder(LibSFDM::library_namespace()+".P2.Quad");

ComponentBuilder<QuadLagrange1D<3>,mesh::ShapeFunction,LibSFDM>
QuadP3_builder(LibSFDM::library_namespace()+".P3.Quad");

ComponentBuilder<QuadLagrange1D<4>,mesh::ShapeFunction,LibSFDM>
QuadP4_builder(LibSFDM::library_namespace()+".P4.Quad");

ComponentBuilder<QuadLagrange1D<5>,mesh::ShapeFunction,LibSFDM>
QuadP5_builder(LibSFDM::library_namespace()+".P5.Quad");

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder<LineLagrange1D<0>,mesh::ShapeFunction,LibSFDM>
LineP0_builder(LibSFDM::library_namespace()+".P0.Line");

ComponentBuilder<LineLagrange1D<1>,mesh::ShapeFunction,LibSFDM>
LineP1_builder(LibSFDM::library_namespace()+".P1.Line");

ComponentBuilder<LineLagrange1D<2>,mesh::ShapeFunction,LibSFDM>
LineP2_builder(LibSFDM::library_namespace()+".P2.Line");

ComponentBuilder<LineLagrange1D<3>,mesh::ShapeFunction,LibSFDM>
LineP3_builder(LibSFDM::library_namespace()+".P3.Line");

ComponentBuilder<LineLagrange1D<4>,mesh::ShapeFunction,LibSFDM>
LineP4_builder(LibSFDM::library_namespace()+".P4.Line");

ComponentBuilder<LineLagrange1D<5>,mesh::ShapeFunction,LibSFDM>
LineP5_builder(LibSFDM::library_namespace()+".P5.Line");

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

