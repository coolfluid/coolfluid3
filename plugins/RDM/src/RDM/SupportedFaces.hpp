// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SupportedFaces_hpp
#define CF_RDM_SupportedFaces_hpp

#include <boost/mpl/vector.hpp>

#include "RDM/Quadrature.hpp"

// 2D

#include "Mesh/SF/Line2DLagrangeP1.hpp"
#include "Mesh/SF/Line2DLagrangeP2.hpp"
#include "Mesh/SF/Line2DLagrangeP3.hpp"

// 3D

#include "Mesh/SF/Triag3DLagrangeP1.hpp"
#include "Mesh/SF/Quad3DLagrangeP1.hpp"

#include "Mesh/Integrators/GaussImplementation.hpp"

#include "RDM/Quadrature.hpp"

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

/// List of supported 2d face shapefunctions
typedef boost::mpl::vector<
  Mesh::SF::Line2DLagrangeP1,
  Mesh::SF::Line2DLagrangeP2,
  Mesh::SF::Line2DLagrangeP3
> FaceTypes2D;


/// List of supported 3d face shapefunctions
typedef boost::mpl::vector<
  Mesh::SF::Triag3DLagrangeP1,
  Mesh::SF::Quad3DLagrangeP1
> FaceTypes3D;

template < int DIM > struct FaceTypes {};

template<> struct FaceTypes<DIM_2D>
{
  typedef FaceTypes2D Faces;
};

template<> struct FaceTypes<DIM_3D>
{
  typedef FaceTypes3D Faces;
};

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------

/// Partial specialization for P1 lines
template <>
struct DefaultQuadrature< Mesh::SF::Line2DLagrangeP1, 1 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 777, Mesh::SF::Line2DLagrangeP1::shape> type;
};

/// Partial specialization for P2 lines
template <>
struct DefaultQuadrature< Mesh::SF::Line2DLagrangeP2, 2 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 8, Mesh::SF::Line2DLagrangeP2::shape> type;
};


/// Partial specialization for P3 lines
template <>
struct DefaultQuadrature< Mesh::SF::Line2DLagrangeP3, 3 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::SF::Line2DLagrangeP3::shape> type;
};

///////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_SupportedFaces_hpp
