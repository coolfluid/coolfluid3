// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_CopySolution_hpp
#define cf3_RDM_CopySolution_hpp

#include "solver/Action.hpp"

#include "RDM/LibRDM.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { class Field; }
namespace RDM {

/// Copies the solution to a set of fields
/// @author Tiago Quintino
class RDM_API CopySolution : public cf3::solver::Action {

public: // typedefs

  /// pointers
  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CopySolution ( const std::string& name );

  /// Virtual destructor
  virtual ~CopySolution() {}

  /// Get the class name
  static std::string type_name () { return "CopySolution"; }

  /// execute the action
  virtual void execute ();

private: // data

  Handle<mesh::Field> m_solution;  ///< solution field pointer

};

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_CopySolution_hpp
