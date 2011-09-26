// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/P2/Line.hpp"

namespace CF {
namespace SFDM {
namespace P2 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Line, Mesh::ShapeFunction, LibSFDM >
  Line_Builder(LibSFDM::library_namespace()+".P2."+Line::type_name());

////////////////////////////////////////////////////////////////////////////////

Line::Line(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;

  const Uint nb_lines = 1;
  const Uint nb_sol_pts_per_line = order+1;
  const Uint nb_flx_pts_per_line = order+2;

  m_points.resize(boost::extents[dimensionality][nb_lines][nb_sol_pts_per_line]);
  m_points[KSI][0][0] = 0;
  m_points[KSI][0][1] = 1;
  m_points[KSI][0][2] = 2;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Line::line() const
{
  static const Line line_sf;
  return line_sf;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Line::flux_line() const
{
  throw Common::NotImplemented(FromHere(),"SFDM::P3::Line not implemented");
  static const Line flux_line_sf;
  return flux_line_sf;
}

////////////////////////////////////////////////////////////////////////////////

void Line::compute_value(const RealVector& local_coordinate, RealRowVector& result) const
{
  const Real ksi2 = local_coordinate[KSI]*local_coordinate[KSI];
  result[0] = 0.5 * (ksi2 - local_coordinate[KSI]);
  result[1] = (1. - ksi2);
  result[2] = 0.5 * (ksi2 + local_coordinate[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void Line::compute_gradient(const RealVector& local_coordinate, RealMatrix& result) const
{
  result(KSI, 0) = local_coordinate[KSI]-0.5;
  result(KSI, 1) = -2.*local_coordinate[KSI];
  result(KSI, 2) = local_coordinate[KSI]+0.5;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Line::local_coordinates() const
{
  static const RealMatrix coords =
      ( RealMatrix(nb_nodes,dimensionality) <<

        -1.,
         0.,
         1.

        ).finished();
  return coords;
}

////////////////////////////////////////////////////////////////////////////////

const Uint Line::nb_nodes;
const Uint Line::dimensionality;
const Uint Line::order;
const Mesh::GeoShape::Type Line::shape;

////////////////////////////////////////////////////////////////////////////////

} // P2
} // SFDM
} // CF
