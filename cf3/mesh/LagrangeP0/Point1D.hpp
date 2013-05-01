// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP0_Point1D_hpp
#define cf3_mesh_LagrangeP0_Point1D_hpp

#include "mesh/ElementTypeBase.hpp"
#include "mesh/LagrangeP0/Point.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP0_API Point1D_traits
{
  typedef Point SF;
  enum { dimension      = 1 };
  enum { nb_faces       = 0 };
  enum { nb_edges       = 0 };
};

/// @brief 1D Lagrange P0 Point Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P0 (constant)
/// point element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP0_API Point1D : public ElementTypeBase<Point1D,Point1D_traits>
{
  static const ElementTypeFaceConnectivity& faces();
  static const ElementType& face_type(const Uint face);

  static Real volume(const NodesT& nodes);
  static Real area(const NodesT& nodes);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);
  static void compute_normal(const NodesT& nodes, CoordsT& normal);
  static bool is_coord_in_element(const CoordsT& coord, const NodesT& nodes);
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP0_Point1D_hpp
