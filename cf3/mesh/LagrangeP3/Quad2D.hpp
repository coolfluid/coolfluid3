// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP3_Quad2D_hpp
#define cf3_mesh_LagrangeP3_Quad2D_hpp

#include "mesh/ElementTypeBase.hpp"
#include "mesh/LagrangeP3/Quad.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP3_API Quad2D_traits
{
  typedef Quad SF;

  enum { dimension      = 2 };
  enum { nb_faces       = 4 };
  enum { nb_edges       = 4 };
};

/// @brief 2D Lagrange P3 Quadrilateral Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P3 (linear)
/// quadrilateral element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP3_API Quad2D : public ElementTypeBase<Quad2D,Quad2D_traits>
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

  static Real area(const NodesT& nodes);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);

  //@}

};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP3_Quad2D_hpp
