// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CGlobalNumbering_hpp
#define CF_Mesh_CGlobalNumbering_hpp

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
/// proc 1:   nodes [ 0 -> 10]   elems [11 -> 20]
/// proc 2:   nodes [21 -> 30]   elems [31 -> 40]
/// proc 3:   nodes [41 -> 50]   elems [51 -> 60]
/// Through the numbering the process it belongs to can be identified:
/// - id 57 must belong to process 3
/// - id 25 must belong to process 2
/// - ...
/// @author Willem Deconinck
class Mesh_Actions_API CGlobalNumbering : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CGlobalNumbering> Ptr;
    typedef boost::shared_ptr<CGlobalNumbering const> ConstPtr;

public: // functions

  /// constructor
  CGlobalNumbering( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CGlobalNumbering"; }

  virtual void execute();

  /// brief description, typically one line
  virtual std::string brief_description() const;

  /// extended help that user can query
  virtual std::string help() const;

public: // functions

  static std::size_t hash_value(const RealMatrix& coords);

private: // data

  bool m_debug;
}; // end CGlobalNumbering


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CGlobalNumbering_hpp
