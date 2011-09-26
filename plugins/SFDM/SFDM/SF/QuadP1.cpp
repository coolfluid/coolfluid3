// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/LibSF.hpp"
#include "SFDM/SF/QuadP1.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < QuadP1, Mesh::ShapeFunction, LibSF > QuadP1_Builder;

////////////////////////////////////////////////////////////////////////////////

QuadP1::QuadP1(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;

  const Uint nb_lines = order+1;
  const Uint nb_sol_pts_per_line = order+1;
  const Uint nb_flx_pts_per_line = order+2;

  m_solution_points.resize(boost::extents[dimensionality][nb_lines][nb_sol_pts_per_line]);
  m_solution_points[KSI][0][0] = 0;
  m_solution_points[KSI][0][1] = 1;
  m_solution_points[KSI][1][0] = 3;
  m_solution_points[KSI][1][1] = 2;
  m_solution_points[ETA][0][0] = 0;
  m_solution_points[ETA][0][1] = 3;
  m_solution_points[ETA][1][0] = 1;
  m_solution_points[ETA][1][1] = 2;

  m_flux_points.resize(boost::extents[dimensionality][nb_lines][nb_flx_pts_per_line]);
  m_flux_points[KSI][0][0] = 0;
  m_flux_points[KSI][0][1] = 1;
  m_flux_points[KSI][0][2] = 2;
  m_flux_points[KSI][1][0] = 6;
  m_flux_points[KSI][1][1] = 7;
  m_flux_points[KSI][1][2] = 8;
  m_flux_points[ETA][0][0] = 0;
  m_flux_points[ETA][0][1] = 3;
  m_flux_points[ETA][0][2] = 6;
  m_flux_points[ETA][1][0] = 2;
  m_flux_points[ETA][1][1] = 5;
  m_flux_points[ETA][1][2] = 8;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& QuadP1::line() const
{
  static const QuadP1 line_sf;
  return line_sf;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& QuadP1::flux_line() const
{
  static const QuadP1 flux_line_sf;
  return flux_line_sf;
}

////////////////////////////////////////////////////////////////////////////////

void QuadP1::compute_value(const RealVector& local_coordinate, RealRowVector& value) const
{
  const Real ksi = local_coordinate[KSI];
  const Real eta = local_coordinate[ETA];

  result[0] = 0.25 * (1.0 - ksi) * (1.0 - eta);
  result[1] = 0.25 * (1.0 + ksi) * (1.0 - eta);
  result[2] = 0.25 * (1.0 + ksi) * (1.0 + eta);
  result[3] = 0.25 * (1.0 - ksi) * (1.0 + eta);
}

////////////////////////////////////////////////////////////////////////////////

void QuadP1::compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const
{
  const Real ksi = local_coordinate[KSI];
  const Real eta = local_coordinate[ETA];

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

const RealMatrix& QuadP1::solution_coordinates() const
{
  static const RealMatrix coords =
      ( RealMatrix(nb_nodes(),dimensionality()) <<

        -1. , -1.,
         1. , -1.,
         1. ,  1.,
        -1. ,  1.

      );
  return coords;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& QuadP1::flux_coordinates() const
{
  static const RealMatrix coords =
      ( RealMatrix(9,dimensionality()) <<

        -1., -1.,
         0., -1.,
         1., -1.,
        -1.,  0.,
         0.,  0.,
         1.,  0.,
        -1.,  1.,
         0.,  1.,
         1.,  1.

      );
  return coords;
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
