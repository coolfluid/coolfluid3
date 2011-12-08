// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_UpdateSolution_hpp
#define cf3_SFDM_UpdateSolution_hpp

#include "solver/Action.hpp"
#include "SFDM/LibSFDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; }
namespace SFDM {

class SFDM_API UpdateSolution : public solver::Action
{
public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  UpdateSolution ( const std::string& name );

  /// Virtual destructor
  virtual ~UpdateSolution() {};

  /// Get the class name
  static std::string type_name () { return "UpdateSolution"; }

  /// execute the action
  virtual void execute ();

private: // functions

  void link_fields();

private: // data

  Handle<mesh::Field> m_solution;
  Handle<mesh::Field> m_residual;
  Handle<mesh::Field> m_jacobian_determinant;
  Handle<mesh::Field> m_update_coeff;
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_UpdateSolution_hpp
