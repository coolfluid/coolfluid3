// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/P3/Quad.hpp"
#include "SFDM/P3/Line.hpp"
#include "SFDM/P4/Line.hpp"

namespace CF {
namespace SFDM {
namespace P3 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Quad, Mesh::ShapeFunction, LibSFDM >
  Quad_Builder(LibSFDM::library_namespace()+".P3."+Quad::type_name());

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
  m_points[KSI][0][3] = 3;
  m_points[KSI][1][0] = 4;
  m_points[KSI][1][1] = 5;
  m_points[KSI][1][2] = 6;
  m_points[KSI][1][3] = 7;
  m_points[KSI][2][0] = 8;
  m_points[KSI][2][1] = 9;
  m_points[KSI][2][2] = 10;
  m_points[KSI][2][3] = 11;
  m_points[KSI][3][0] = 12;
  m_points[KSI][3][1] = 13;
  m_points[KSI][3][2] = 14;
  m_points[KSI][3][3] = 15;
  m_points[ETA][0][0] = 0;
  m_points[ETA][0][1] = 4;
  m_points[ETA][0][2] = 8;
  m_points[ETA][0][3] = 12;
  m_points[ETA][1][0] = 1;
  m_points[ETA][1][1] = 5;
  m_points[ETA][1][2] = 9;
  m_points[ETA][1][3] = 13;
  m_points[ETA][2][0] = 2;
  m_points[ETA][2][1] = 6;
  m_points[ETA][2][2] = 10;
  m_points[ETA][2][3] = 14;
  m_points[ETA][3][0] = 3;
  m_points[ETA][3][1] = 7;
  m_points[ETA][3][2] = 11;
  m_points[ETA][3][3] = 15;

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
  const static ShapeFunction::ConstPtr line_sf(Common::allocate_component< P3::Line >(P3::Line::type_name()));
  return *line_sf;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Quad::flux_line() const
{
  const static ShapeFunction::ConstPtr flux_line_sf(Common::allocate_component< P4::Line >(P4::Line::type_name()));
  return *flux_line_sf;
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_value(const RealVector& local_coordinate, RealRowVector& result) const
{
  const Real ksi = local_coordinate[KSI];
  const Real eta = local_coordinate[ETA];
  const Real eta2 = eta*eta;
  const Real ksi2 = ksi*ksi;
  const Real sqrt3 = sqrt(3.);

  result[0]  =  ((-1. + eta)*(-1. + 3.*eta2)*(-1. + ksi)*(-1. + 3.*ksi2))/16.;
  result[1]  =  (sqrt3*(-1. + eta)*(-1. + 3.*eta2)*(sqrt3 - 3.*ksi)*(-1. + ksi2))/16.;
  result[2]  =  (sqrt3*(-1. + eta)*(-1. + 3.*eta2)*(sqrt3 + 3.*ksi)*(-1. + ksi2))/16.;
  result[3]  = -((-1. + eta)*(-1. + 3.*eta2)*(1. + ksi)*(-1. + 3.*ksi2))/16.;
  result[4]  =  (sqrt3*(sqrt3 - 3*eta)*(-1 + eta2)*(-1 + ksi)*(-1 + 3*ksi2))/16.;
  result[5]  =  (3.*(sqrt3 - 3.*eta)*(-1. + eta)*(1. + eta)*(sqrt3 - 3.*ksi)*(-1. + ksi)*(1. + ksi))/16.;
  result[6]  =  (3.*(sqrt3 - 3.*eta)*(-1. + eta)*(1. + eta)*(sqrt3 + 3.*ksi)*(-1. + ksi)*(1. + ksi))/16.;
  result[7]  = -(sqrt3*(sqrt3 - 3.*eta)*(-1. + eta2)*( 1. + ksi)*(-1. + 3.*ksi2))/16.;
  result[8]  =  (sqrt3*(sqrt3 + 3.*eta)*(-1. + eta2)*(-1. + ksi)*(-1. + 3.*ksi2))/16.;
  result[9]  =  (3.*(-1. + eta)*(1. + eta)*(sqrt3 + 3.*eta)*(sqrt3 - 3.*ksi)*(-1. + ksi)*(1. + ksi))/16.;
  result[10] =  (3.*(-1. + eta)*(1. + eta)*(sqrt3 + 3.*eta)*(sqrt3 + 3.*ksi)*(-1. + ksi)*(1. + ksi))/16.;
  result[11] = -(sqrt3*(sqrt3 + 3.*eta)*(-1. + eta2)*(1. + ksi)*(-1. + 3.*ksi2))/16.;
  result[12] = -((1. + eta)*(-1. + 3*eta2)*(-1 + ksi)*(-1 + 3*ksi2))/16.;
  result[13] = -(sqrt3*(1. + eta)*(-1. + 3.*eta2)*(sqrt3 - 3.*ksi)*(-1. + ksi2))/16.;
  result[14] = -(sqrt3*(1. + eta)*(-1. + 3.*eta2)*(sqrt3 + 3.*ksi)*(-1. + ksi2))/16.;
  result[15] =  ((1. + eta)*(-1. + 3.*eta2)*(1. + ksi)*(-1. + 3.*ksi2))/16.;
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_gradient(const RealVector& local_coordinate, RealMatrix& result) const
{
  const Real ksi = local_coordinate[KSI];
  const Real eta = local_coordinate[ETA];
  const Real eta2 = eta*eta;
  const Real ksi2 = ksi*ksi;
  const Real sqrt3 = sqrt(3.);

  result(KSI, 0) = ((-1 + eta)*(-1 + 3*eta2)*(-1 - 6*ksi + 9*ksi2))/16.;
  result(ETA, 0) = ((-1 - 6*eta + 9*eta2)*(-1 + ksi)*(-1 + 3*ksi2))/16.;
  result(KSI, 1) = (-3*(-1 + eta)*(-1 + 3*eta2)*(-sqrt3 + ksi*(-2 + 3*sqrt3*ksi)))/16.;
  result(ETA, 1) = (-3*(-1 - 6*eta + 9*eta2)*(-1 + sqrt3*ksi)*(-1 + ksi2))/16.;
  result(KSI, 2) = (3*(-1 + eta)*(-1 + 3*eta2)*(-sqrt3 + ksi*(2 + 3*sqrt3*ksi)))/16.;
  result(ETA, 2) = (3*(-1 - 6*eta + 9*eta2)*(1 + sqrt3*ksi)*(-1 + ksi2))/16.;
  result(KSI, 3) = -((-1 + eta)*(-1 + 3*eta2)*(-1 + 6*ksi + 9*ksi2))/16.;
  result(ETA, 3) = -((-1 - 6*eta + 9*eta2)*(1 + ksi)*(-1 + 3*ksi2))/16.;
  result(KSI, 4) = (-3*(-1 + sqrt3*eta)*(-1 + eta2)*(-1 - 6*ksi + 9*ksi2))/16.;
  result(ETA, 4) = (-3*(-sqrt3 + eta*(-2 + 3*sqrt3*eta))*(-1 + ksi)*(-1 + 3*ksi2))/16.;
  result(KSI, 5) = (3*(sqrt3 - 3*eta)*(-1 + eta)*(1 + eta)*(3 + 2*sqrt3*ksi - 9*ksi2))/16.;
  result(ETA, 5) = (3*(3 + 2*sqrt3*eta - 9*eta2)*(sqrt3 - 3*ksi)*(-1 + ksi)*(1 + ksi))/16.;
  result(KSI, 6) = (3*(sqrt3 - 3*eta)*(-1 + eta)*(1 + eta)*(-3 + 2*sqrt3*ksi + 9*ksi2))/16.;
  result(ETA, 6) = (3*(3 + 2*sqrt3*eta - 9*eta2)*(-1 + ksi)*(1 + ksi)*(sqrt3 + 3*ksi))/16.;
  result(KSI, 7) = (3*(-1 + sqrt3*eta)*(-1 + eta2)*(-1 + 6*ksi + 9*ksi2))/16.;
  result(ETA, 7) = (3*(-sqrt3 + eta*(-2 + 3*sqrt3*eta))*(1 + ksi)*(-1 + 3*ksi2))/16.;
  result(KSI, 8) = (3*(1 + sqrt3*eta)*(-1 + eta2)*(-1 - 6*ksi + 9*ksi2))/16.;
  result(ETA, 8) = (3*(-sqrt3 + eta*(2 + 3*sqrt3*eta))*(-1 + ksi)*(-1 + 3*ksi2))/16.;
  result(KSI, 9) = (3*(-1 + eta)*(1 + eta)*(sqrt3 + 3*eta)*(3 + 2*sqrt3*ksi - 9*ksi2))/16.;
  result(ETA, 9) = (3*(-3 + 2*sqrt3*eta + 9*eta2)*(sqrt3 - 3*ksi)*(-1 + ksi)*(1 + ksi))/16.;
  result(KSI, 10) = (3*(-1 + eta)*(1 + eta)*(sqrt3 + 3*eta)*(-3 + 2*sqrt3*ksi + 9*ksi2))/16.;
  result(ETA, 10) = (3*(-3 + 2*sqrt3*eta + 9*eta2)*(-1 + ksi)*(1 + ksi)*(sqrt3 + 3*ksi))/16.;
  result(KSI, 11) = (-3*(1 + sqrt3*eta)*(-1 + eta2)*(-1 + 6*ksi + 9*ksi2))/16.;
  result(ETA, 11) = (-3*(-sqrt3 + eta*(2 + 3*sqrt3*eta))*(1 + ksi)*(-1 + 3*ksi2))/16.;
  result(KSI, 12) = -((1 + eta)*(-1 + 3*eta2)*(-1 - 6*ksi + 9*ksi2))/16.;
  result(ETA, 12) = -((-1 + 6*eta + 9*eta2)*(-1 + ksi)*(-1 + 3*ksi2))/16.;
  result(KSI, 13) = (3*(1 + eta)*(-1 + 3*eta2)*(-sqrt3 + ksi*(-2 + 3*sqrt3*ksi)))/16.;
  result(ETA, 13) = (3*(-1 + 6*eta + 9*eta2)*(-1 + sqrt3*ksi)*(-1 + ksi2))/16.;
  result(KSI, 14) = (-3*(1 + eta)*(-1 + 3*eta2)*(-sqrt3 + ksi*(2 + 3*sqrt3*ksi)))/16.;
  result(ETA, 14) = (-3*(-1 + 6*eta + 9*eta2)*(1 + sqrt3*ksi)*(-1 + ksi2))/16.;
  result(KSI, 15) = ((1 + eta)*(-1 + 3*eta2)*(-1 + 6*ksi + 9*ksi2))/16.;
  result(ETA, 15) = ((-1 + 6*eta + 9*eta2)*(1 + ksi)*(-1 + 3*ksi2))/16.;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Quad::local_coordinates() const
{
  static const RealMatrix coords =
      ( RealMatrix(nb_nodes,dimensionality) <<

        -1.,          -1.,
        -1./sqrt(3.), -1.,
         1./sqrt(3.), -1.,
         1.,          -1.,
        -1.,          -1./sqrt(3.),
        -1./sqrt(3.), -1./sqrt(3.),
         1./sqrt(3.), -1./sqrt(3.),
         1.,          -1./sqrt(3.),
        -1.,           1./sqrt(3.),
        -1./sqrt(3.),  1./sqrt(3.),
         1./sqrt(3.),  1./sqrt(3.),
         1.,           1./sqrt(3.),
        -1.,           1.,
        -1./sqrt(3.),  1.,
         1./sqrt(3.),  1.,
         1.,           1.

        ).finished();
  return coords;
}

////////////////////////////////////////////////////////////////////////////////

const Uint Quad::nb_nodes;
const Uint Quad::dimensionality;
const Uint Quad::order;
const Mesh::GeoShape::Type Quad::shape;

////////////////////////////////////////////////////////////////////////////////

} // P3
} // SFDM
} // CF
