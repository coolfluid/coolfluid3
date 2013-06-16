// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_vtk_Tetrahedralize_hpp
#define CF_vtk_Tetrahedralize_hpp

#include "mesh/MeshTransformer.hpp"

#include "vtk/LibVTK.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace vtk {

////////////////////////////////////////////////////////////////////////////////

/// Apply a Delaunay triangulation to the point cloud in a mesh and remove all other existing volume regions, if any.
/// This results in a pure tetrahedral mesh filling the convex hull of the points. No surface regions are created.
class Tetrahedralize : public mesh::MeshTransformer
{
public:
  Tetrahedralize ( const std::string& name );
  virtual ~Tetrahedralize();
  
  static std::string type_name () { return "Tetrahedralize"; }
  
  virtual void execute();
};
  
////////////////////////////////////////////////////////////////////////////////

} //  vtk
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif /* CF_vtk_Tetrahedralize_hpp */
