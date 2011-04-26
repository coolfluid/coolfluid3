// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SupportedTypes_hpp
#define CF_RDM_SupportedTypes_hpp

#include <boost/mpl/vector.hpp>

#include "Mesh/CElements.hpp"

// 2D

#include "Mesh/SF/Line2DLagrangeP1.hpp"
#include "Mesh/SF/Line2DLagrangeP2.hpp"
#include "Mesh/SF/Line2DLagrangeP3.hpp"

#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Mesh/SF/Triag2DLagrangeP2.hpp"
#include "Mesh/SF/Triag2DLagrangeP2B.hpp"
#include "Mesh/SF/Triag2DLagrangeP3.hpp"

#include "Mesh/SF/Quad2DLagrangeP1.hpp"
#include "Mesh/SF/Quad2DLagrangeP2.hpp"
#include "Mesh/SF/Quad2DLagrangeP3.hpp"

// 3D

#include "Mesh/SF/Triag3DLagrangeP1.hpp"
#include "Mesh/SF/Quad3DLagrangeP1.hpp"

#include "Mesh/SF/Tetra3DLagrangeP1.hpp"
#include "Mesh/SF/Hexa3DLagrangeP1.hpp"


#include "Mesh/Integrators/GaussImplementation.hpp"

namespace CF {
namespace RDM {

//------------------------------------------------------------------------------------------

/// List of supported 2d face shapefunctions
typedef boost::mpl::vector<
  Mesh::SF::Line2DLagrangeP1,
  Mesh::SF::Line2DLagrangeP2,
  Mesh::SF::Line2DLagrangeP3
> FaceTypes2D;

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

/// List of supported 3d face shapefunctions
typedef boost::mpl::vector<
  Mesh::SF::Triag3DLagrangeP1,
  Mesh::SF::Quad3DLagrangeP1
> FaceTypes3D;

/// List of supported 3d cell shapefunctions
typedef boost::mpl::vector<
  Mesh::SF::Tetra3DLagrangeP1,
  Mesh::SF::Hexa3DLagrangeP1
> CellTypes3D;

template < int DIM > struct ElemTypes {};

template<>
struct ElemTypes<DIM_2D>
{
  typedef CellTypes2D Cells;
  typedef FaceTypes2D Faces;
};

template<>
struct ElemTypes<DIM_3D>
{
  typedef CellTypes3D Cells;
  typedef FaceTypes3D Faces;
};

//------------------------------------------------------------------------------------------

/// Predicate class to test if the region contains a specific element type
template < typename TYPE >
struct IsElementType
{
  bool operator()(const Mesh::CElements& component)
  {
    return Mesh::IsElementType<TYPE>()( component.element_type() );
  }
};

template < typename SF, Uint order = SF::order >
struct DefaultQuadrature
{
  typedef Mesh::Integrators::GaussMappedCoords< order, SF::shape> type;
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

//------------------------------------------------------------------------------------------

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

/// Partial specialization for P1 triangles
template <>
struct DefaultQuadrature< Mesh::SF::Triag2DLagrangeP1, 1 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::SF::Triag2DLagrangeP2::shape> type;
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

//------------------ FOR TESTING ----------------------------------
#if 0
/// Partial specialization for P1 triangles
template <>
struct DefaultQuadrature< Mesh::SF::Triag2DLagrangeP1, 1 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::SF::Triag2DLagrangeP1::shape> type;
};

/// Partial specialization for P1 quadrilaterals
template <>
struct DefaultQuadrature< Mesh::SF::Quad2DLagrangeP1, 1 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 2, Mesh::SF::Quad2DLagrangeP1::shape> type;
};

/// Partial specialization for P2 quadrilaterals
template <>
struct DefaultQuadrature< Mesh::SF::Quad2DLagrangeP2, 2 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::SF::Quad2DLagrangeP3::shape> type;
};

#endif

///////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_SupportedTypes_hpp
