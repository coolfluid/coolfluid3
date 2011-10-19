// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/P0/Quad.hpp"
#include "SFDM/P0/Line.hpp"
#include "SFDM/P1/Line.hpp"

namespace cf3 {
namespace SFDM {
namespace P0 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Quad, Mesh::ShapeFunction, LibSFDM >
  Quad_Builder(LibSFDM::library_namespace()+".P0."+Quad::type_name());

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
  m_points[ETA][0][0] = 0;

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
  m_face_number[KSI][LEFT]=KSI_NEG;
  m_face_number[KSI][RIGHT]=KSI_POS;
  m_face_number[ETA][LEFT]=ETA_NEG;
  m_face_number[ETA][RIGHT]=ETA_POS;

}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Quad::line() const
{
  const static ShapeFunction::ConstPtr line_sf(common::allocate_component< P0::Line >(P0::Line::type_name()));
  return *line_sf;
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& Quad::flux_line() const
{
  const static ShapeFunction::ConstPtr flux_line_sf(common::allocate_component< P1::Line >(P1::Line::type_name()));
  return *flux_line_sf;
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_value(const RealVector& local_coordinate, RealRowVector& result) const
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_gradient(const RealVector& local_coordinate, RealMatrix& result) const
{
  result(KSI, 0) = 0.;
  result(ETA, 0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Quad::local_coordinates() const
{
  static const RealMatrix coords =
      ( RealMatrix(nb_nodes,dimensionality) <<

        0. , 0.

        ).finished();
  return coords;
}

////////////////////////////////////////////////////////////////////////////////

const Uint Quad::nb_nodes;
const Uint Quad::dimensionality;
const Uint Quad::order;
const Mesh::GeoShape::Type Quad::shape;

////////////////////////////////////////////////////////////////////////////////

} // P0
} // SFDM
} // cf3
