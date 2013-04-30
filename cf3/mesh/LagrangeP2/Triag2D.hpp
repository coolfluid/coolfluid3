// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP2_Triag2D_hpp
#define cf3_mesh_LagrangeP2_Triag2D_hpp

#include "mesh/ElementTypeBase.hpp"
#include "mesh/LagrangeP2/Triag.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP2_API Triag2D_traits
{
  typedef Triag SF;

  enum { dimension      = 2 };
  enum { nb_faces       = 3 };
  enum { nb_edges       = 3 };
};

/// @brief 2D Lagrange P2 Triangular Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P2 (linear)
/// triangular element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP2_API Triag2D : public ElementTypeBase<Triag2D,Triag2D_traits>
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

  //@}

};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP2_Triag2D_hpp
