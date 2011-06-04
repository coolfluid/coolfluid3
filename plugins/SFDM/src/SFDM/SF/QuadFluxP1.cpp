// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/LibSF.hpp"
#include "SFDM/SF/QuadFluxP1.hpp"
#include "SFDM/SF/LineFluxP1.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < QuadFluxP1, Mesh::ShapeFunction, LibSF > QuadFluxP1_Builder;

////////////////////////////////////////////////////////////////////////////////

QuadFluxP1::QuadFluxP1(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
  m_nb_lines_per_orientation = nb_lines_per_orientation;

  m_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][nb_nodes_per_line]);
  m_points[KSI][0][0] = 0;
  m_points[KSI][0][1] = 1;
  m_points[ETA][0][0] = 2;
  m_points[ETA][0][1] = 3;

  m_face_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][2]);
  m_face_points[KSI][0][LEFT]  = 0;
  m_face_points[KSI][0][RIGHT] = 1;
  m_face_points[ETA][0][LEFT]  = 2;
  m_face_points[ETA][0][RIGHT] = 3;

  m_face_number.resize(boost::extents[nb_orientations][2]);
  m_face_number[KSI][LEFT ]=3;
  m_face_number[KSI][RIGHT]=1;
  m_face_number[ETA][LEFT ]=0;
  m_face_number[ETA][RIGHT]=2;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& QuadFluxP1::line() const
{
  return line_type();
}

////////////////////////////////////////////////////////////////////////////////

const LineFluxP1& QuadFluxP1::line_type()
{
  static const LineFluxP1 sf;
  return sf;
}

////////////////////////////////////////////////////////////////////////////////

void QuadFluxP1::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  throw Common::NotImplemented(FromHere(),"This should never be called, as fluxes are only being computed using LineFluxP1 instead.");
}

////////////////////////////////////////////////////////////////////////////////

void QuadFluxP1::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  throw Common::NotImplemented(FromHere(),"This should never be called, as fluxes are only being computed using LineFluxP1 instead.");
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix QuadFluxP1::s_mapped_sf_nodes =  ( RealMatrix(4,2) <<
  -1.,  0.,
   1.,  0.,
   0., -1.,
   0.,  1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
