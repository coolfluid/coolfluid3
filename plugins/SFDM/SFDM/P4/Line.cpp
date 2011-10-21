// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "SFDM/P4/Line.hpp"

namespace cf3 {
namespace SFDM {
namespace P4 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Line, mesh::ShapeFunction, LibSFDM >
  Line_Builder(LibSFDM::library_namespace()+".P4."+Line::type_name());

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
  m_points[KSI][0][3] = 3;
  m_points[KSI][0][4] = 4;

  m_face_info.resize(boost::extents[2][2]);
  m_face_info[KSI_NEG][ORIENTATION] = KSI;
  m_face_info[KSI_NEG][SIDE] = NEG;
  m_face_info[KSI_POS][ORIENTATION] = KSI;
  m_face_info[KSI_POS][SIDE] = POS;

  m_face_number.resize(boost::extents[dimensionality][2]);
  m_face_number[KSI][LEFT ]=0;
  m_face_number[KSI][RIGHT]=1;
}

////////////////////////////////////////////////////////////////////////////////


const SFDM::ShapeFunction& Line::line() const
{
  const static SFDM::ShapeFunction::Ptr line_sf(common::allocate_component< P4::Line >(P4::Line::type_name()));
  return *line_sf;
}

////////////////////////////////////////////////////////////////////////////////

const SFDM::ShapeFunction& Line::flux_line() const
{
  throw common::NotImplemented(FromHere(),"SFDM::P5::Line not implemented");
  const static SFDM::ShapeFunction::ConstPtr flux_line_sf(common::allocate_component< Line >(Line::type_name()));
  return *flux_line_sf;
}

////////////////////////////////////////////////////////////////////////////////

void Line::compute_value(const RealVector& local_coordinate, RealRowVector& result) const
{
  const Real ksi = local_coordinate[KSI];
  const Real ksi2 = ksi*ksi;
  const Real sqrt15 = sqrt(15.);

  result[0] = ((-1. + ksi)*ksi*(-3. + 5.*ksi2))/4.;
  result[1] = (5.*(sqrt15 - 5.*ksi)*ksi*(-1. + ksi2))/12.;
  result[2] = (3. - 8.*ksi2 + 5.*ksi2*ksi2)/3.;
  result[3] = (-5.*(sqrt15 + 5.*ksi)*ksi*(-1. + ksi2))/12.;
  result[4] = ((1. + ksi)*ksi*(-3. + 5.*ksi2))/4.;
}

////////////////////////////////////////////////////////////////////////////////

void Line::compute_gradient(const RealVector& local_coordinate, RealMatrix& result) const
{
  const Real ksi = local_coordinate[KSI];
  const Real ksi2 = ksi*ksi;
  const Real sqrt15 = sqrt(15.);

  result(KSI, 0) = (3. + ksi*(-6. + 5.*ksi*(-3. + 4.*ksi)))/4.;
  result(KSI, 1) = (5.*(-sqrt15 + ksi*(10. + 3.*sqrt15*ksi - 20.*ksi2)))/12.;
  result(KSI, 2) = (4.*ksi*(-4. + 5.*ksi2))/3.;
  result(KSI, 3) = (5.*(sqrt15 + ksi*(10. - 3.*sqrt15*ksi - 20.*ksi2)))/12.;
  result(KSI, 4) = (-3. + ksi*(-6. + 5.*ksi*(3. + 4.*ksi)))/4.;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Line::local_coordinates() const
{
  static const RealMatrix coords =
      ( RealMatrix(nb_nodes,dimensionality) <<

        -1,
        -sqrt(3./5.),
         0,
         sqrt(3./5.),
         1

        ).finished();
  return coords;
}

////////////////////////////////////////////////////////////////////////////////

const Uint Line::nb_nodes;
const Uint Line::dimensionality;
const Uint Line::order;
const mesh::GeoShape::Type Line::shape;

////////////////////////////////////////////////////////////////////////////////

} // P4
} // SFDM
} // cf3
