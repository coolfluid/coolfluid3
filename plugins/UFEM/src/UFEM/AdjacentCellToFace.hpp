// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_AdjacentCellToFace_hpp
#define cf3_UFEM_AdjacentCellToFace_hpp


#include "solver/Action.hpp"

#include "LibUFEM.hpp"

namespace cf3 {
  namespace mesh { class CNodeConnectivity; }
namespace UFEM {

/// Copy field values from elements adjacent to a surface patch to the corresponding surface patch
/// Useful to store values from element fields in the boundary elements for later use
class UFEM_API AdjacentCellToFace : public solver::Action
{

public: // functions

  /// Contructor
  /// @param name of the component
  AdjacentCellToFace ( const std::string& name );

  virtual ~AdjacentCellToFace();

  /// Get the class name
  static std::string type_name () { return "AdjacentCellToFace"; }

  virtual void execute();

private:
  virtual void on_regions_set();

  Handle<mesh::CNodeConnectivity> m_node_connectivity;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_AdjacentCellToFace_hpp
