// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_SupportedFaces_hpp
#define cf3_RDM_SupportedFaces_hpp

#include <boost/mpl/vector.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/back_inserter.hpp>

#include "RDM/Quadrature.hpp"

// 2D

#include "Mesh/LagrangeP1/Line2D.hpp"
#include "Mesh/LagrangeP2/Line2D.hpp"
#include "Mesh/LagrangeP3/Line2D.hpp"

// 3D

#include "Mesh/LagrangeP1/Triag3D.hpp"
#include "Mesh/LagrangeP1/Quad3D.hpp"

#include "Mesh/Integrators/GaussImplementation.hpp"

#include "RDM/Quadrature.hpp"

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

/// List of supported 2d face shapefunctions
typedef boost::mpl::vector<
  Mesh::LagrangeP1::Line2D,
  Mesh::LagrangeP2::Line2D,
  Mesh::LagrangeP3::Line2D
> FaceTypes2D;


/// List of supported 3d face shapefunctions
typedef boost::mpl::vector<
  Mesh::LagrangeP1::Triag3D,
  Mesh::LagrangeP1::Quad3D
> FaceTypes3D;

typedef boost::mpl::copy< FaceTypes2D, boost::mpl::back_inserter< FaceTypes3D > >::type AllFaceTypes;

//------------------------------------------------------------------------------------------

template < int DIM > struct FaceTypes {};  ///< selects element types according to dimension

template<> struct FaceTypes<DIM_2D>
{
  typedef FaceTypes2D Faces;
};

template<> struct FaceTypes<DIM_3D>
{
  typedef FaceTypes3D Faces;
};

//------------------------------------------------------------------------------------------

/// Partial specialization for P1 lines
template <>
struct DefaultQuadrature< Mesh::LagrangeP1::Line2D, 1 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 777, Mesh::LagrangeP1::Line2D::shape> type;
};

/// Partial specialization for P2 lines
template <>
struct DefaultQuadrature< Mesh::LagrangeP2::Line2D, 2 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 8, Mesh::LagrangeP2::Line2D::shape> type;
};


/// Partial specialization for P3 lines
template <>
struct DefaultQuadrature< Mesh::LagrangeP3::Line2D, 3 >
{
  typedef Mesh::Integrators::GaussMappedCoords< 4, Mesh::LagrangeP3::Line2D::shape> type;
};

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_SupportedFaces_hpp
