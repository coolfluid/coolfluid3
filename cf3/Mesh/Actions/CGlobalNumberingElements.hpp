// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_Actions_CGlobalNumberingElementsElements_hpp
#define cf3_Mesh_Actions_CGlobalNumberingElementsElements_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Create a global number for nodes and elements of the mesh
///
/// Global numbers are created using a hash based on the coordinates
/// of the nodes, and all vertex coordinates of the elements.
/// After numbering the nodes and elements will share the global numbering
/// table given an example for 30 elements on 3 processes
/// proc 1:   elems [ 0 -> 9]
/// proc 2:   elems [10 -> 19]
/// proc 3:   nodes [20 -> 29]
/// Through the numbering the process it belongs to can be identified:
/// - id 25 must belong to process 3
/// - id 12 must belong to process 2
/// - ...
/// @author Willem Deconinck
class Mesh_Actions_API CGlobalNumberingElements : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CGlobalNumberingElements> Ptr;
    typedef boost::shared_ptr<CGlobalNumberingElements const> ConstPtr;

public: // functions

  /// constructor
  CGlobalNumberingElements( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CGlobalNumberingElements"; }

  virtual void execute();

  /// brief description, typically one line
  virtual std::string brief_description() const;

  /// extended help that user can query
  virtual std::string help() const;

private: // functions

  std::size_t hash_value(const RealMatrix& coords);

  bool m_debug;
}; // end CGlobalNumberingElements


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Mesh_Actions_CGlobalNumberingElements_hpp
