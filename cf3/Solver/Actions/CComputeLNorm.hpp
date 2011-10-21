// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Actions_CComputeLNorm_hpp
#define cf3_Solver_Actions_CComputeLNorm_hpp

#include "common/Action.hpp"

#include "Solver/Actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; }
namespace Solver {
namespace Actions {

class Solver_Actions_API CComputeLNorm : public common::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CComputeLNorm> Ptr;
  typedef boost::shared_ptr<CComputeLNorm const> ConstPtr;

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

private: // data

  boost::weak_ptr<mesh::Field> m_field;

};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

#endif // cf3_Solver_Actions_CComputeLNorm_hpp
