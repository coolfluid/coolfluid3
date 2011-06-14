// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Actions_CGlobalNumberingNodes_hpp
#define CF_Mesh_Actions_CGlobalNumberingNodes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Create a global number for nodes and elements of the mesh
///
/// Global numbers are created using a hash based on the coordinates
/// of the nodes, and all vertex coordinates of the elements.
/// After numbering the nodes and elements will share the global numbering
/// table given an example with 30 nodes and 30 elements on 3 processes
/// proc 1:   nodes [ 0 ->  9]
/// proc 2:   nodes [10 -> 19]
/// proc 3:   nodes [20 -> 29]
/// Through the numbering the process it belongs to can be identified:
/// - id 25 must belong to process 3
/// - id 11 must belong to process 2
/// - ...
/// @author Willem Deconinck
class Mesh_Actions_API CGlobalNumberingNodes : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CGlobalNumberingNodes> Ptr;
    typedef boost::shared_ptr<CGlobalNumberingNodes const> ConstPtr;

public: // functions

  /// constructor
  CGlobalNumberingNodes( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CGlobalNumberingNodes"; }

  virtual void execute();

  /// brief description, typically one line
  virtual std::string brief_description() const;

  /// extended help that user can query
  virtual std::string help() const;

private: // functions

  std::size_t hash_value(const RealVector& coords);

  bool m_debug;
}; // end CGlobalNumberingNodes


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Actions_CGlobalNumberingNodes_hpp
