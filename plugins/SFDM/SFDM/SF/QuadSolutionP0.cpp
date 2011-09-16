// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/QuadSolutionP0.hpp"
#include "SFDM/SF/LineSolutionP0.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < QuadSolutionP0, Mesh::ShapeFunction, LibSF > QuadSolutionP0_Builder;

////////////////////////////////////////////////////////////////////////////////

QuadSolutionP0::QuadSolutionP0(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
  m_nb_lines_per_orientation = nb_lines_per_orientation;

  m_points.resize(boost::extents[nb_orientations][nb_lines_per_orientation][nb_nodes_per_line]);
  m_points[KSI][0][0] = 0;
  m_points[ETA][0][0] = 0;

  m_face_number.resize(boost::extents[nb_orientations][2]);
  m_face_number[KSI][LEFT ]=3;
  m_face_number[KSI][RIGHT]=1;
  m_face_number[ETA][LEFT ]=0;
  m_face_number[ETA][RIGHT]=2;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& QuadSolutionP0::line() const
{
  return line_type();
}

////////////////////////////////////////////////////////////////////////////////

const LineSolutionP0& QuadSolutionP0::line_type()
{
  static const LineSolutionP0 sf;
  return sf;
}

////////////////////////////////////////////////////////////////////////////////

void QuadSolutionP0::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void QuadSolutionP0::compute_gradient(const MappedCoordsT& mappedCoord, GradientT& result)
{
  result(KSI, 0) = 0.;
  result(ETA, 0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix QuadSolutionP0::s_mapped_sf_nodes =  ( RealMatrix(1,2) <<
    0. , 0.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
