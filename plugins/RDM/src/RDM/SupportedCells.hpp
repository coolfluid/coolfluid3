// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_SupportedCells_hpp
#define cf3_RDM_SupportedCells_hpp

#include <boost/mpl/vector.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/back_inserter.hpp>

#include "RDM/Quadrature.hpp"

// 2D

#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/LagrangeP2/Triag2D.hpp"
#include "mesh/LagrangeP2B/Triag2D.hpp"
#include "mesh/LagrangeP3/Triag2D.hpp"

#include "mesh/LagrangeP1/Quad2D.hpp"
#include "mesh/LagrangeP2/Quad2D.hpp"
#include "mesh/LagrangeP3/Quad2D.hpp"

// 3D

#include "mesh/LagrangeP1/Tetra3D.hpp"
#include "mesh/LagrangeP1/Hexa3D.hpp"


#include "mesh/Integrators/GaussImplementation.hpp"

#include "RDM/Quadrature.hpp"

namespace cf3 {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

//#define RDM_ALL_CELLS

#ifdef RDM_ALL_CELLS
/// List of supported 2d cell shapefunctions
typedef boost::mpl::vector<
  mesh::LagrangeP1::Triag2D,
  mesh::LagrangeP2::Triag2D,
  mesh::LagrangeP2B::Triag2D,
  mesh::LagrangeP3::Triag2D,
  mesh::LagrangeP1::Quad2D,
  mesh::LagrangeP2::Quad2D,
  mesh::LagrangeP3::Quad2D
> CellTypes2D;

/// List of supported 3d cell shapefunctions
typedef boost::mpl::vector<
  mesh::LagrangeP1::Tetra3D,
  mesh::LagrangeP1::Hexa3D
> CellTypes3D;

#else

typedef boost::mpl::vector<
  mesh::LagrangeP2::Triag2D,
  mesh::LagrangeP1::Triag2D,
  mesh::LagrangeP2B::Triag2D
> CellTypes2D;
typedef boost::mpl::vector<mesh::LagrangeP1::Tetra3D> CellTypes3D;

#endif

typedef boost::mpl::copy< CellTypes2D, boost::mpl::back_inserter< CellTypes3D > >::type AllCellTypes;

//------------------------------------------------------------------------------------------

template < int DIM > struct CellTypes {};  ///< selects element types according to dimension

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
struct DefaultQuadrature< mesh::LagrangeP1::Triag2D, 1 >
{
  typedef mesh::Integrators::GaussMappedCoords< 4, mesh::LagrangeP1::Triag2D::shape> type;
};

/// Partial specialization for P2 triangles
template <>
struct DefaultQuadrature< mesh::LagrangeP2::Triag2D, 2 >
{
  typedef mesh::Integrators::GaussMappedCoords< 4, mesh::LagrangeP2::Triag2D::shape> type;
};

/// Partial specialization for P1 quadrilaterals
template <>
struct DefaultQuadrature< mesh::LagrangeP1::Quad2D, 1 >
{
  typedef mesh::Integrators::GaussMappedCoords< 4, mesh::LagrangeP1::Quad2D::shape> type;
};

/// Partial specialization for P2 quadrilaterals
template <>
struct DefaultQuadrature< mesh::LagrangeP2::Quad2D, 2 >
{
  typedef mesh::Integrators::GaussMappedCoords< 4, mesh::LagrangeP2::Quad2D::shape> type;
};

/// Partial specialization for P3 triangles
template <>
struct DefaultQuadrature< mesh::LagrangeP3::Triag2D, 3 >
{
  typedef mesh::Integrators::GaussMappedCoords< 5, mesh::LagrangeP3::Triag2D::shape> type;
};

/// Partial specialization for P3 quadrilaterals
template <>
struct DefaultQuadrature< mesh::LagrangeP3::Quad2D, 3 >
{
  typedef mesh::Integrators::GaussMappedCoords< 8, mesh::LagrangeP3::Quad2D::shape> type;
};

//------------------------------------------------------------------------------------------

/// Partial specialization for P2 with bubble.
/// Standard second order integration uses only boundary quadrature points,
/// where bubble function is zero, thus has uncoupled modes.
template <>
struct DefaultQuadrature< mesh::LagrangeP2B::Triag2D, 2 >
{
  typedef mesh::Integrators::GaussMappedCoords< 4, mesh::LagrangeP2B::Triag2D::shape> type;
};

///////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_SupportedCells_hpp
