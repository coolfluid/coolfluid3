// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SupportedCells_hpp
#define CF_RDM_SupportedCells_hpp

#include <boost/mpl/vector.hpp>

#include "RDM/Core/Quadrature.hpp"

// 2D

#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Mesh/SF/Triag2DLagrangeP2.hpp"
#include "Mesh/SF/Triag2DLagrangeP2B.hpp"
#include "Mesh/SF/Triag2DLagrangeP3.hpp"

#include "Mesh/SF/Quad2DLagrangeP1.hpp"
#include "Mesh/SF/Quad2DLagrangeP2.hpp"
#include "Mesh/SF/Quad2DLagrangeP3.hpp"

// 3D

#include "Mesh/SF/Tetra3DLagrangeP1.hpp"
#include "Mesh/SF/Hexa3DLagrangeP1.hpp"


#include "Mesh/Integrators/GaussImplementation.hpp"

#include "RDM/Core/Quadrature.hpp"

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

/// List of supported 2d cell shapefunctions
typedef boost::mpl::vector<
  Mesh::SF::Triag2DLagrangeP1,
  Mesh::SF::Triag2DLagrangeP2,
  Mesh::SF::Triag2DLagrangeP2B,
  Mesh::SF::Triag2DLagrangeP3,
  Mesh::SF::Quad2DLagrangeP1,
  Mesh::SF::Quad2DLagrangeP2,
  Mesh::SF::Quad2DLagrangeP3
> CellTypes2D;

/// List of supported 3d cell shapefunctions
typedef boost::mpl::vector<
  Mesh::SF::Tetra3DLagrangeP1,
  Mesh::SF::Hexa3DLagrangeP1
> CellTypes3D;

//------------------------------------------------------------------------------------------

template < int DIM > struct CellTypes {};

template<> struct CellTypes<DIM_2D>
{
  typedef CellTypes2D Cells;
};

template<> struct CellTypes<DIM_3D>
{
  typedef CellTypes3D Cells;
};

//------------------------------------------------------------------------------------------

/// Partial specialization for P1 triangles
template <>
struct DefaultQuadrature< Mesh::SF::Triag2DLagrangeP1, 1 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::SF::Triag2DLagrangeP1::shape> type;
};

/// Partial specialization for P2 triangles
template <>
struct DefaultQuadrature< Mesh::SF::Triag2DLagrangeP2, 2 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::SF::Triag2DLagrangeP2::shape> type;
};

/// Partial specialization for P1 quadrilaterals
template <>
struct DefaultQuadrature< Mesh::SF::Quad2DLagrangeP1, 1 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::SF::Quad2DLagrangeP1::shape> type;
};

/// Partial specialization for P2 quadrilaterals
template <>
struct DefaultQuadrature< Mesh::SF::Quad2DLagrangeP2, 2 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::SF::Quad2DLagrangeP2::shape> type;
};

/// Partial specialization for P3 triangles
template <>
struct DefaultQuadrature< Mesh::SF::Triag2DLagrangeP3, 3 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 5, Mesh::SF::Triag2DLagrangeP3::shape> type;
};

/// Partial specialization for P3 quadrilaterals
template <>
struct DefaultQuadrature< Mesh::SF::Quad2DLagrangeP3, 3 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 8, Mesh::SF::Quad2DLagrangeP3::shape> type;
};

//------------------------------------------------------------------------------------------

/// Partial specialization for P2 with bubble.
/// Standard second order integration uses only boundary quadrature points,
/// where bubble function is zero, thus has uncoupled modes.
template <>
struct DefaultQuadrature< Mesh::SF::Triag2DLagrangeP2B, 2 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::SF::Triag2DLagrangeP2B::shape> type;
};

///////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_SupportedCells_hpp
