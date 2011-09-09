// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP2B_Triag2D_hpp
#define CF_Mesh_LagrangeP2B_Triag2D_hpp

#include "Mesh/ElementTypeBase.hpp"
#include "Mesh/LagrangeP2B/Triag.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP2B {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP2B_API Triag2D_traits
{
  typedef Triag SF;

  enum { dimension      = 2 };
  enum { nb_faces       = 3 };
  enum { nb_edges       = 3 };
};


/// @brief 2D Lagrange P2B Triangular Element type
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P2B (linear)
/// triangular element.
/// @see ElementType for documentation of undocumented functions
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct Mesh_LagrangeP2B_API Triag2D : public ElementTypeBase<Triag2D,Triag2D_traits>
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

} // LagrangeP2B
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP2B_Triag2D_hpp
