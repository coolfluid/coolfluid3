// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/QuadSolutionP1.hpp"
#include "SFDM/SF/LineSolutionP1.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < QuadSolutionP1, Mesh::ShapeFunction, LibSF > QuadSolutionP1_Builder;

////////////////////////////////////////////////////////////////////////////////

QuadSolutionP1::QuadSolutionP1(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
  m_nb_lines_per_orientation = nb_lines_per_orientation;

  m_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][nb_nodes_per_line]);
  m_points[KSI][0][0] = 0;
  m_points[KSI][0][1] = 1;
  m_points[KSI][1][0] = 3;
  m_points[KSI][1][1] = 2;
  m_points[ETA][0][0] = 0;
  m_points[ETA][0][1] = 3;
  m_points[ETA][1][0] = 1;
  m_points[ETA][1][1] = 2;

  m_face_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][2]);
  m_face_points[KSI][0][0] = 0;
  m_face_points[KSI][0][1] = 1;
  m_face_points[KSI][1][0] = 3;
  m_face_points[KSI][1][1] = 2;
  m_face_points[ETA][0][0] = 0;
  m_face_points[ETA][0][1] = 3;
  m_face_points[ETA][1][0] = 1;
  m_face_points[ETA][1][1] = 2;

  m_face_number.resize(boost::extents[nb_orientations][2]);
  m_face_number[KSI][LEFT ]=3;
  m_face_number[KSI][RIGHT]=1;
  m_face_number[ETA][LEFT ]=0;
  m_face_number[ETA][RIGHT]=2;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& QuadSolutionP1::line() const
{
  return line_type();
}

////////////////////////////////////////////////////////////////////////////////

const LineSolutionP1& QuadSolutionP1::line_type()
{
  static const LineSolutionP1 sf;
  return sf;
}

////////////////////////////////////////////////////////////////////////////////

void QuadSolutionP1::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real ksi  = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result[0] = 0.25 * (1.0 - ksi) * (1.0 - eta);
  result[1] = 0.25 * (1.0 + ksi) * (1.0 - eta);
  result[2] = 0.25 * (1.0 + ksi) * (1.0 + eta);
  result[3] = 0.25 * (1.0 - ksi) * (1.0 + eta);
}

////////////////////////////////////////////////////////////////////////////////

void QuadSolutionP1::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result(KSI, 0) = 0.25 * (-1. + eta);
  result(ETA, 0) = 0.25 * (-1. + ksi);
  result(KSI, 1) = 0.25 * ( 1. - eta);
  result(ETA, 1) = 0.25 * (-1. - ksi);
  result(KSI, 2) = 0.25 * ( 1. + eta);
  result(ETA, 2) = 0.25 * ( 1. + ksi);
  result(KSI, 3) = 0.25 * (-1. - eta);
  result(ETA, 3) = 0.25 * ( 1. - ksi);
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix QuadSolutionP1::s_mapped_sf_nodes =  ( RealMatrix(4,2) <<
    -1. , -1.,
     1. , -1.,
     1. ,  1.,
    -1. ,  1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
