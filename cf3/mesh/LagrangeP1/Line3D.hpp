// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP1_Line3D_hpp
#define cf3_mesh_LagrangeP1_Line3D_hpp

#include "mesh/ElementTypeBase.hpp"
#include "mesh/LagrangeP1/Line.hpp"
#include "mesh/ElementType.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP1_API Line3D_traits
{
  typedef Line SF;

  enum { dimension      = 3 };
  enum { nb_faces       = 2 };
  enum { nb_edges       = 2 };
};


/// @brief Lagrange P1 Triangular Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// triangular element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP1_API Line3D : public ElementTypeBase<Line3D,Line3D_traits>
{
  /// @name Accessor functions
  //  ------------------------
  //@{

  static const ElementTypeFaceConnectivity& faces();
  static const ElementType& face_type(const Uint face);

  //@}

  /// @name Computation functions
  //  ---------------------------
  //@{

  static JacobianT jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes);
  static void compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& jacobian);
  static Real volume(const NodesT& nodes);
  static Real area(const NodesT& nodes);
  static Real length(const NodesT& nodes);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);

  //@}

};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP1_Line3D_hpp
