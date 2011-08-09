// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_GhostCells_hpp
#define CF_FVM_Core_GhostCells_hpp

////////////////////////////////////////////////////////////////////////////////

#include "FVM/Core/LibCore.hpp"
#include "Mesh/CCells.hpp"

namespace CF {
namespace Mesh {
  class Geometry;
}
namespace FVM {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

/// GhostCells component class
/// The ghost cell derives from Mesh::CCells, to mark it as a cell, while it is
/// in fact a Point with zero dimensionality. This is to store the ghost states
/// linked to a ghost cell. Since the ghost cell is not actually defined, but 
/// represented by the coordinate of the mirrored centroid, its element type
/// must be a Point type.
/// @author Willem Deconinck, 
class FVM_Core_API GhostCells : public Mesh::CCells 
{
public: // typedefs

  typedef boost::shared_ptr<GhostCells> Ptr;
  typedef boost::shared_ptr<GhostCells const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  GhostCells ( const std::string& name );

  /// Initialize the GhostCells using the given type
  virtual void initialize(const std::string& element_type_name, Mesh::Geometry& nodes);
    
  /// Virtual destructor
  virtual ~GhostCells();

  /// Get the class name
  static std::string type_name () { return "GhostCells"; }

};

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_GhostCells_hpp
