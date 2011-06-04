// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/LineFluxP2.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < LineFluxP2, Mesh::ShapeFunction, LibSF > LineFluxP2_Builder;

////////////////////////////////////////////////////////////////////////////////

LineFluxP2::LineFluxP2(const std::string& name) : ShapeFunction(name)
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

  m_face_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][2]);
  m_face_points[KSI][0][LEFT]  = 0;
  m_face_points[KSI][0][RIGHT] = 2;

  m_face_number.resize(boost::extents[nb_orientations][2]);
  m_face_number[KSI][LEFT ]=0;
  m_face_number[KSI][RIGHT]=1;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& LineFluxP2::line() const
{
  return line_type();
}

////////////////////////////////////////////////////////////////////////////////

const LineFluxP2& LineFluxP2::line_type()
{
  static const LineFluxP2 sf;
  return sf;
}

////////////////////////////////////////////////////////////////////////////////

void LineFluxP2::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real ksi2 = mapped_coord[KSI]*mapped_coord[KSI];
  result[0] = 0.5 * (ksi2 - mapped_coord[KSI]);
  result[1] = (1. - ksi2);
  result[2] = 0.5 * (ksi2 + mapped_coord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void LineFluxP2::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = mapped_coord[KSI]-0.5;
  result(KSI, 1) = -2.*mapped_coord[KSI];
  result(KSI, 2) = mapped_coord[KSI]+0.5;

}

////////////////////////////////////////////////////////////////////////////////

RealMatrix LineFluxP2::s_mapped_sf_nodes =  ( RealMatrix(3,1) <<
  -1.,
   0.,
   1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
