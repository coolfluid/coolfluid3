// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/LineFluxP3.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < LineFluxP3, Mesh::ShapeFunction, LibSF > LineFluxP3_Builder;

////////////////////////////////////////////////////////////////////////////////

LineFluxP3::LineFluxP3(const std::string& name) : ShapeFunction(name)
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
  m_points[KSI][0][3] = 3;

  m_face_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][2]);
  m_face_points[KSI][0][LEFT]  = 0;
  m_face_points[KSI][0][RIGHT] = 3;

}

////////////////////////////////////////////////////////////////////////////////

void LineFluxP3::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real ksi2 = mapped_coord[KSI]*mapped_coord[KSI];
  const Real sqrt3 = sqrt(3.);

  result[0] =  0.25 * ( 1. - ksi) * (-1. + 3*ksi2);
  result[1] =  0.75 * (-1. + sqrt3*ksi) * (-1. + ksi2);
  result[2] =  0.75 * (-1. - sqrt3*ksi) * (-1. + ksi2);
  result[3] =  0.25 * ( 1. + ksi) * (-1. + 3*ksi2);
}

////////////////////////////////////////////////////////////////////////////////

void LineFluxP3::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real ksi2 = mapped_coord[KSI]*mapped_coord[KSI];
  const Real sqrt3 = sqrt(3.);

  result(KSI, 0) =  0.25 * ( 1. + 6.*ksi - 9.*ksi2);
  result(KSI, 1) = -0.75 * ( sqrt3 + ksi*(2.-3.*sqrt3*ksi));
  result(KSI, 2) = -0.75 * (-sqrt3 + ksi*(2.+3.*sqrt3*ksi));
  result(KSI, 3) =  0.25 * (-1. + 6.*ksi + 9.*ksi2);
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix LineFluxP3::s_mapped_sf_nodes =  ( RealMatrix(4,1) <<
  -1.,
  -1./sqrt(3.),
   1./sqrt(3.),
   1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
