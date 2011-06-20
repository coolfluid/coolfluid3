// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/QuadSolutionP2.hpp"
#include "SFDM/SF/LineSolutionP2.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < QuadSolutionP2, Mesh::ShapeFunction, LibSF > QuadSolutionP2_Builder;

////////////////////////////////////////////////////////////////////////////////

QuadSolutionP2::QuadSolutionP2(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
  m_nb_lines_per_orientation = nb_lines_per_orientation;

  m_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][nb_nodes_per_line]);
  m_points[KSI][0][0] = 0;
  m_points[KSI][0][1] = 1;
  m_points[KSI][0][2] = 2;
  m_points[KSI][1][0] = 3;
  m_points[KSI][1][1] = 4;
  m_points[KSI][1][2] = 5;
  m_points[KSI][2][0] = 6;
  m_points[KSI][2][1] = 7;
  m_points[KSI][2][2] = 8;
  m_points[ETA][0][0] = 0;
  m_points[ETA][0][1] = 3;
  m_points[ETA][0][2] = 6;
  m_points[ETA][1][0] = 1;
  m_points[ETA][1][1] = 4;
  m_points[ETA][1][2] = 7;
  m_points[ETA][2][0] = 2;
  m_points[ETA][2][1] = 5;
  m_points[ETA][2][2] = 8;

  m_face_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][2]);
  m_face_points[KSI][0][LEFT ] = 0;
  m_face_points[KSI][0][RIGHT] = 2;
  m_face_points[KSI][1][LEFT ] = 3;
  m_face_points[KSI][1][RIGHT] = 5;
  m_face_points[KSI][2][LEFT ] = 6;
  m_face_points[KSI][2][RIGHT] = 8;
  m_face_points[ETA][0][LEFT ] = 0;
  m_face_points[ETA][0][RIGHT] = 6;
  m_face_points[ETA][1][LEFT ] = 1;
  m_face_points[ETA][1][RIGHT] = 7;
  m_face_points[ETA][2][LEFT ] = 2;
  m_face_points[ETA][2][RIGHT] = 8;

  m_face_number.resize(boost::extents[nb_orientations][2]);
  m_face_number[KSI][LEFT ]=3;
  m_face_number[KSI][RIGHT]=1;
  m_face_number[ETA][LEFT ]=0;
  m_face_number[ETA][RIGHT]=2;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& QuadSolutionP2::line() const
{
  return line_type();
}

////////////////////////////////////////////////////////////////////////////////

const LineSolutionP2& QuadSolutionP2::line_type()
{
  static const LineSolutionP2 sf;
  return sf;
}

////////////////////////////////////////////////////////////////////////////////

void QuadSolutionP2::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];
  const Real eta2 = eta*eta;
  const Real ksi2 = ksi*ksi;

  result[0] =  ((-1 + eta)*eta*(-1 + ksi)*ksi)*0.25;
  result[1] = -((-1 + eta)*eta*(-1 + ksi2))*0.5;
  result[2] =  ((-1 + eta)*eta*ksi*(1 + ksi))*0.25;
  result[3] = -((-1 + eta2)*(-1 + ksi)*ksi)*0.5;
  result[4] =  (-1 + eta2)*(-1 + ksi2);
  result[5] = -((-1 + eta2)*ksi*(1 + ksi))*0.5;
  result[6] =  (eta*(1 + eta)*(-1 + ksi)*ksi)*0.25;
  result[7] = -(eta*(1 + eta)*(-1 + ksi2))*0.5;
  result[8] =  (eta*(1 + eta)*ksi*(1 + ksi))*0.25;
}

////////////////////////////////////////////////////////////////////////////////

void QuadSolutionP2::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];
  const Real eta2 = eta*eta;
  const Real ksi2 = ksi*ksi;

  result(KSI, 0) = ((-1 + eta)*eta*(-1 + 2*ksi))*0.25;
  result(ETA, 0) = ((-1 + 2*eta)*(-1 + ksi)*ksi)*0.25;
  result(KSI, 1) = -((-1 + eta)*eta*ksi);
  result(ETA, 1) = -((-1 + 2*eta)*(-1 + ksi2))*0.5;
  result(KSI, 2) = ((-1 + eta)*eta*(1 + 2*ksi))*0.25;
  result(ETA, 2) = ((-1 + 2*eta)*ksi*(1 + ksi))*0.25;
  result(KSI, 3) = -((-1 + eta2)*(-1 + 2*ksi))*0.5;
  result(ETA, 3) = -(eta*(-1 + ksi)*ksi);
  result(KSI, 4) = 2*(-1 + eta2)*ksi;
  result(ETA, 4) = 2*eta*(-1 + ksi2);
  result(KSI, 5) = -((-1 + eta2)*(1 + 2*ksi))*0.5;
  result(ETA, 5) = -(eta*ksi*(1 + ksi));
  result(KSI, 6) = (eta*(1 + eta)*(-1 + 2*ksi))*0.25;
  result(ETA, 6) = ((1 + 2*eta)*(-1 + ksi)*ksi)*0.25;
  result(KSI, 7) = -(eta*(1 + eta)*ksi);
  result(ETA, 7) = -((1 + 2*eta)*(-1 + ksi2))*0.5;
  result(KSI, 8) = (eta*(1 + eta)*(1 + 2*ksi))*0.25;
  result(ETA, 8) = ((1 + 2*eta)*ksi*(1 + ksi))*0.25;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix QuadSolutionP2::s_mapped_sf_nodes =  ( RealMatrix(9,2) <<
    -1. , -1.,
     0. , -1.,
     1. , -1.,
    -1. ,  0.,
     0. ,  0.,
     1. ,  0.,
    -1. ,  1.,
     0. ,  1.,
     1. ,  1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
