// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP2_Line2D_hpp
#define cf3_mesh_LagrangeP2_Line2D_hpp

#include "mesh/ElementTypeBase.hpp"
#include "mesh/LagrangeP2/Line.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP2_API Line2D_traits
{
  typedef Line SF;
  enum { dimension      = 2 };
  enum { nb_faces       = 1 };
  enum { nb_edges       = 2 };
};

/// @brief Lagrange P2 Triangular Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P2 (linear)
/// triangular element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP2_API Line2D : public ElementTypeBase<Line2D,Line2D_traits>
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

  static Real volume(const NodesT& nodes);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);
  static void compute_normal(const NodesT& nodes , CoordsT& normal);

  //@}

};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP2_Line2D_hpp
