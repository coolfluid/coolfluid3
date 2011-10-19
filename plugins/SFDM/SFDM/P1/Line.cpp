// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"

#include "SFDM/P1/Line.hpp"
#include "SFDM/P2/Line.hpp"

namespace cf3 {
namespace SFDM {
namespace P1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Line, Mesh::ShapeFunction, LibSFDM >
  Line_Builder(LibSFDM::library_namespace()+".P1."+Line::type_name());

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

  m_face_info.resize(boost::extents[2][2]);
  m_face_info[KSI_NEG][ORIENTATION] = KSI;
  m_face_info[KSI_NEG][SIDE] = NEG;
  m_face_info[KSI_POS][ORIENTATION] = KSI;
  m_face_info[KSI_POS][SIDE] = POS;

  m_face_number.resize(boost::extents[dimensionality][2]);
  m_face_number[KSI][LEFT]=0;
  m_face_number[KSI][RIGHT]=1;
}

////////////////////////////////////////////////////////////////////////////////

const SFDM::ShapeFunction& Line::line() const
{
  const static SFDM::ShapeFunction::Ptr line_sf(common::allocate_component< P1::Line >(P1::Line::type_name()));
  return *line_sf;
}

////////////////////////////////////////////////////////////////////////////////

const SFDM::ShapeFunction& Line::flux_line() const
{
  const static SFDM::ShapeFunction::ConstPtr flux_line_sf(common::allocate_component< P2::Line >(P2::Line::type_name()));
  return *flux_line_sf;
}


////////////////////////////////////////////////////////////////////////////////

void Line::compute_value(const RealVector& local_coordinate, RealRowVector& result) const
{
  result[0] = 0.5 * (1.0 - local_coordinate[KSI]);
  result[1] = 0.5 * (1.0 + local_coordinate[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void Line::compute_gradient(const RealVector& local_coordinate, RealMatrix& result) const
{
  result(KSI, 0) = -0.5;
  result(KSI, 1) =  0.5;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Line::local_coordinates() const
{
  static const RealMatrix coords =
      ( RealMatrix(nb_nodes,dimensionality) <<

        -1.,
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

} // P1
} // SFDM
} // cf3
