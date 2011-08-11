// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_GrowOverlap_hpp
#define CF_Mesh_GrowOverlap_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Grow the overlap of the mesh with one layer
///
/// Boundary nodes of one rank are communicated to other ranks.
/// Each other rank then communicates all elements that are connected
/// to these boundary nodes.
/// Missing nodes are then also communicated to complete the elements
///
/// @author Willem Deconinck
class Mesh_Actions_API GrowOverlap : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<GrowOverlap> Ptr;
    typedef boost::shared_ptr<GrowOverlap const> ConstPtr;

public: // functions

  /// constructor
  GrowOverlap( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "GrowOverlap"; }

  virtual void execute();

}; // end GrowOverlap


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_GrowOverlap_hpp
