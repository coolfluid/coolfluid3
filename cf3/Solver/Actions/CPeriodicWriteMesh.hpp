// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Actions_CPeriodicWriteMesh_hpp
#define cf3_Solver_Actions_CPeriodicWriteMesh_hpp

#include "Solver/Actions/LibActions.hpp"
#include "Solver/Action.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; class CMesh; class WriteMesh; }
namespace Solver {
namespace Actions {

class Solver_Actions_API CPeriodicWriteMesh : public Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CPeriodicWriteMesh> Ptr;
  typedef boost::shared_ptr<CPeriodicWriteMesh const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CPeriodicWriteMesh ( const std::string& name );

  /// Virtual destructor
  virtual ~CPeriodicWriteMesh() {}

  /// Get the class name
  static std::string type_name () { return "CPeriodicWriteMesh"; }

  /// execute the action
  virtual void execute ();

private: // data

  boost::weak_ptr<Component> m_iterator;  ///< component that holds the iteration

  mesh::WriteMesh& m_writer; ///< mesh writer

};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

#endif // cf3_Solver_Actions_CPeriodicWriteMesh_hpp
