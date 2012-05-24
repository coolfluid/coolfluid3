// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/PointInterpolatorT.hpp"
#include "mesh/Interpolator.hpp"
#include "mesh/ElementFinderOcttree.hpp"
#include "mesh/StencilComputerOcttree.hpp"
#include "mesh/StencilComputerRings.hpp"
#include "mesh/PseudoLaplacianLinearInterpolation.hpp"
#include "mesh/ShapeFunctionInterpolation.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

// PseudoLaplacianLinearPointInterpolator for linear interpolation of one point, using a stencil with neighbouring cells
typedef PointInterpolatorT<ElementFinderOcttree,StencilComputerRings,PseudoLaplacianLinearInterpolation> PseudoLaplacianLinearPointInterpolator;
ComponentBuilder< PseudoLaplacianLinearPointInterpolator , APointInterpolator, LibMesh>
  PseudoLaplacianLinearPointInterpolator_builder(LibMesh::library_namespace()+".PseudoLaplacianLinearPointInterpolator");

// PseudoLaplacianLinearInterpolator using PseudoLaplacianLinearPointInterpolator
typedef InterpolatorT<PseudoLaplacianLinearPointInterpolator> PseudoLaplacianLinearInterpolator;
ComponentBuilder< PseudoLaplacianLinearInterpolator , AInterpolator, LibMesh>
  PseudoLaplacianLinearInterpolator_builder(LibMesh::library_namespace()+".PseudoLaplacianLinearInterpolator");

////////////////////////////////////////////////////////////////////////////////

// ShapeFunctionPointInterpolator for exact interpolation of one point, using a finite-element shapefunction
typedef PointInterpolatorT<ElementFinderOcttree,StencilComputerOneCell,ShapeFunctionInterpolation> ShapeFunctionPointInterpolator;
ComponentBuilder< PointInterpolatorT<ElementFinderOcttree,StencilComputerOneCell,ShapeFunctionInterpolation> , APointInterpolator, LibMesh>
  ShapeFunctionPointInterpolator_builder(LibMesh::library_namespace()+".ShapeFunctionPointInterpolator");

// ShapeFunctionInterpolator using ShapeFunctionPointInterpolator
typedef InterpolatorT<ShapeFunctionPointInterpolator> ShapeFunctionInterpolator;
ComponentBuilder< ShapeFunctionInterpolator , AInterpolator, LibMesh>
  ShapeFunctionInterpolator_builder(LibMesh::library_namespace()+".ShapeFunctionInterpolator");

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
