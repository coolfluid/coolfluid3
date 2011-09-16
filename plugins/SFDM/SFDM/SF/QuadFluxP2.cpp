// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/LibSF.hpp"
#include "SFDM/SF/QuadFluxP2.hpp"
#include "SFDM/SF/LineFluxP2.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < QuadFluxP2, Mesh::ShapeFunction, LibSF > QuadFluxP2_Builder;

////////////////////////////////////////////////////////////////////////////////

QuadFluxP2::QuadFluxP2(const std::string& name) : ShapeFunction(name)
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
  m_points[KSI][1][0] = 6;
  m_points[KSI][1][1] = 7;
  m_points[KSI][1][2] = 8;
  m_points[ETA][0][0] = 0;
  m_points[ETA][0][1] = 3;
  m_points[ETA][0][2] = 6;
  m_points[ETA][1][0] = 2;
  m_points[ETA][1][1] = 5;
  m_points[ETA][1][2] = 8;

  m_face_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][2]);
  m_face_points[KSI][0][LEFT]  = 0;
  m_face_points[KSI][0][RIGHT] = 2;
  m_face_points[KSI][1][LEFT]  = 6;
  m_face_points[KSI][1][RIGHT] = 8;;
  m_face_points[ETA][0][LEFT]  = 0;
  m_face_points[ETA][0][RIGHT] = 6;
  m_face_points[ETA][1][LEFT]  = 2;
  m_face_points[ETA][1][RIGHT] = 8;

  m_face_number.resize(boost::extents[nb_orientations][2]);
  m_face_number[KSI][LEFT ]=3;
  m_face_number[KSI][RIGHT]=1;
  m_face_number[ETA][LEFT ]=0;
  m_face_number[ETA][RIGHT]=2;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& QuadFluxP2::line() const
{
  return line_type();
}

////////////////////////////////////////////////////////////////////////////////

const LineFluxP2& QuadFluxP2::line_type()
{
  static const LineFluxP2 sf;
  return sf;
}

////////////////////////////////////////////////////////////////////////////////

void QuadFluxP2::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  throw Common::NotImplemented(FromHere(),"This should never be called, as fluxes are only being computed using LineFluxP2 instead.");
}

////////////////////////////////////////////////////////////////////////////////

void QuadFluxP2::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  throw Common::NotImplemented(FromHere(),"This should never be called, as fluxes are only being computed using LineFluxP2 instead.");
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix QuadFluxP2::s_mapped_sf_nodes =  ( RealMatrix(9,2) <<
  -1., -1.,
   0., -1.,
   1., -1.,
  -1.,  0.,
   0.,  0.,
   1.,  0.,
  -1.,  1.,
   0.,  1.,
   1.,  1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
