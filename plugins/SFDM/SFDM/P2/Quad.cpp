// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "SFDM/P2/Quad.hpp"
#include "SFDM/P2/Line.hpp"
#include "SFDM/P3/Line.hpp"

namespace cf3 {
namespace SFDM {
namespace P2 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Quad, mesh::ShapeFunction, LibSFDM >
  Quad_Builder(LibSFDM::library_namespace()+".P2."+Quad::type_name());

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

  m_face_info.resize(boost::extents[4][2]);
  m_face_info[KSI_NEG][ORIENTATION] = KSI;
  m_face_info[KSI_NEG][SIDE] = NEG;
  m_face_info[KSI_POS][ORIENTATION] = KSI;
  m_face_info[KSI_POS][SIDE] = POS;
  m_face_info[ETA_NEG][ORIENTATION] = ETA;
  m_face_info[ETA_NEG][SIDE] = NEG;
  m_face_info[ETA_POS][ORIENTATION] = ETA;
  m_face_info[ETA_POS][SIDE] = POS;

  m_face_number.resize(boost::extents[dimensionality][2]);
  m_face_number[KSI][LEFT ]=KSI_NEG;
  m_face_number[KSI][RIGHT]=KSI_POS;
  m_face_number[ETA][LEFT ]=ETA_NEG;
  m_face_number[ETA][RIGHT]=ETA_POS;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Quad::line() const
{
  const static boost::shared_ptr< ShapeFunction > line_sf(common::allocate_component< P2::Line >(P2::Line::type_name()));
  return *line_sf;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Quad::flux_line() const
{
  const static boost::shared_ptr< ShapeFunction > flux_line_sf(common::allocate_component< P3::Line >(P2::Line::type_name()));
  return *flux_line_sf;
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_value(const RealVector& local_coordinate, RealRowVector& result) const
{
  const Real ksi = local_coordinate[KSI];
  const Real eta = local_coordinate[ETA];
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

void Quad::compute_gradient(const RealVector& local_coordinate, RealMatrix& result) const
{
  const Real ksi = local_coordinate[KSI];
  const Real eta = local_coordinate[ETA];
  const Real eta2 = eta*eta;
  const Real ksi2 = ksi*ksi;

  result(KSI, 0) =  ((-1 + eta)*eta*(-1 + 2*ksi))*0.25;
  result(ETA, 0) =  ((-1 + 2*eta)*(-1 + ksi)*ksi)*0.25;
  result(KSI, 1) = -((-1 + eta)*eta*ksi);
  result(ETA, 1) = -((-1 + 2*eta)*(-1 + ksi2))*0.5;
  result(KSI, 2) =  ((-1 + eta)*eta*(1 + 2*ksi))*0.25;
  result(ETA, 2) =  ((-1 + 2*eta)*ksi*(1 + ksi))*0.25;
  result(KSI, 3) = -((-1 + eta2)*(-1 + 2*ksi))*0.5;
  result(ETA, 3) = -(eta*(-1 + ksi)*ksi);
  result(KSI, 4) =  2*(-1 + eta2)*ksi;
  result(ETA, 4) =  2*eta*(-1 + ksi2);
  result(KSI, 5) = -((-1 + eta2)*(1 + 2*ksi))*0.5;
  result(ETA, 5) = -(eta*ksi*(1 + ksi));
  result(KSI, 6) =  (eta*(1 + eta)*(-1 + 2*ksi))*0.25;
  result(ETA, 6) =  ((1 + 2*eta)*(-1 + ksi)*ksi)*0.25;
  result(KSI, 7) = -(eta*(1 + eta)*ksi);
  result(ETA, 7) = -((1 + 2*eta)*(-1 + ksi2))*0.5;
  result(KSI, 8) =  (eta*(1 + eta)*(1 + 2*ksi))*0.25;
  result(ETA, 8) =  ((1 + 2*eta)*ksi*(1 + ksi))*0.25;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Quad::local_coordinates() const
{
  static const RealMatrix coords =
      ( RealMatrix(nb_nodes,dimensionality) <<

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
const mesh::GeoShape::Type Quad::shape;

////////////////////////////////////////////////////////////////////////////////

} // P2
} // SFDM
} // cf3
