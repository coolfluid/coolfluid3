// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/P1/Quad.hpp"
#include "SFDM/P1/Line.hpp"
#include "SFDM/P2/Line.hpp"

namespace CF {
namespace SFDM {
namespace P1 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Quad, Mesh::ShapeFunction, LibSFDM >
  Quad_Builder(LibSFDM::library_namespace()+".P1."+Quad::type_name());

////////////////////////////////////////////////////////////////////////////////

Quad::Quad(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;

  const Uint nb_lines = order+1;
  const Uint nb_sol_pts_per_line = order+1;
  const Uint nb_flx_pts_per_line = order+2;

  m_points.resize(boost::extents[dimensionality][nb_lines][nb_sol_pts_per_line]);
  m_points[KSI][0][0] = 0;
  m_points[KSI][0][1] = 1;
  m_points[KSI][1][0] = 3;
  m_points[KSI][1][1] = 2;
  m_points[ETA][0][0] = 0;
  m_points[ETA][0][1] = 3;
  m_points[ETA][1][0] = 1;
  m_points[ETA][1][1] = 2;

  m_face_info.resize(boost::extents[4][2]);
  m_face_info[KSI_NEG][ORIENTATION] = KSI;
  m_face_info[KSI_NEG][SIDE] = NEG;
  m_face_info[KSI_POS][ORIENTATION] = KSI;
  m_face_info[KSI_POS][SIDE] = POS;
  m_face_info[ETA_NEG][ORIENTATION] = ETA;
  m_face_info[ETA_NEG][SIDE] = NEG;
  m_face_info[ETA_POS][ORIENTATION] = ETA;
  m_face_info[ETA_POS][SIDE] = POS;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Quad::line() const
{
  const static ShapeFunction::ConstPtr line_sf(Common::allocate_component< P1::Line >(P1::Line::type_name()));
  return *line_sf;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Quad::flux_line() const
{
  const static ShapeFunction::ConstPtr flux_line_sf(Common::allocate_component< P2::Line >(P2::Line::type_name()));
  return *flux_line_sf;
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_value(const RealVector& local_coordinate, RealRowVector& result) const
{
  const Real ksi = local_coordinate[KSI];
  const Real eta = local_coordinate[ETA];

  result[0] = 0.25 * (1.0 - ksi) * (1.0 - eta);
  result[1] = 0.25 * (1.0 + ksi) * (1.0 - eta);
  result[2] = 0.25 * (1.0 + ksi) * (1.0 + eta);
  result[3] = 0.25 * (1.0 - ksi) * (1.0 + eta);
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_gradient(const RealVector& local_coordinate, RealMatrix& result) const
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

const RealMatrix& Quad::local_coordinates() const
{
  static const RealMatrix coords =
      ( RealMatrix(nb_nodes,dimensionality) <<

        -1. , -1.,
         1. , -1.,
         1. ,  1.,
        -1. ,  1.

        ).finished();
  return coords;
}

////////////////////////////////////////////////////////////////////////////////

//const RealMatrix& Quad::flux_coordinates() const
//{
//  static const RealMatrix coords =
//      ( RealMatrix(9,dimensionality) <<

//        -1., -1.,
//         0., -1.,
//         1., -1.,
//        -1.,  0.,
//         0.,  0.,
//         1.,  0.,
//        -1.,  1.,
//         0.,  1.,
//         1.,  1.

//        ).finished();
//  return coords;
//}

////////////////////////////////////////////////////////////////////////////////

const Uint Quad::nb_nodes;
const Uint Quad::dimensionality;
const Uint Quad::order;
const Mesh::GeoShape::Type Quad::shape;

////////////////////////////////////////////////////////////////////////////////

} // P1
} // SFDM
} // CF
