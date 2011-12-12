// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CComputeLNorm_hpp
#define cf3_solver_actions_CComputeLNorm_hpp

#include "common/Action.hpp"

#include "solver/actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; }
namespace solver {
namespace actions {

class solver_actions_API CComputeLNorm : public common::Action {

public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  CComputeLNorm ( const std::string& name );

  /// Virtual destructor
  virtual ~CComputeLNorm() {}

  /// Get the class name
  static std::string type_name () { return "CComputeLNorm"; }

  /// execute the action
  virtual void execute ();

  Real compute_norm(mesh::Field&) const;

private: // data

//  boost::weak_ptr<mesh::Field> m_field;

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

#endif // cf3_solver_actions_CComputeLNorm_hpp
