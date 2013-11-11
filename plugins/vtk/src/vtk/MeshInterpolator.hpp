// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_vtk_MeshInterpolator_hpp
#define CF_vtk_MeshInterpolator_hpp

#include "common/Action.hpp"

#include "vtk/LibVTK.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace vtk {

////////////////////////////////////////////////////////////////////////////////

/// Interpolate one mesh to another. Geometry dict only.
class MeshInterpolator : public common::Action
{
public:
  MeshInterpolator ( const std::string& name );
  virtual ~MeshInterpolator();
  
  static std::string type_name () { return "MeshInterpolator"; }
  
  virtual void execute();
};
  
////////////////////////////////////////////////////////////////////////////////

} //  vtk
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif /* CF_vtk_MeshInterpolator_hpp */
