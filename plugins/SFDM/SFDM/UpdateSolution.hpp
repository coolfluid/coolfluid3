// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_UpdateSolution_hpp
#define cf3_SFDM_UpdateSolution_hpp

#include "Solver/Action.hpp"
#include "SFDM/LibSFDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh   { class Field; }
namespace SFDM {

class SFDM_API UpdateSolution : public Solver::Action
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<UpdateSolution> Ptr;
  typedef boost::shared_ptr<UpdateSolution const> ConstPtr;

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

  boost::weak_ptr<Mesh::Field> m_solution;
  boost::weak_ptr<Mesh::Field> m_residual;
  boost::weak_ptr<Mesh::Field> m_update_coeff;
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF3_SFDM_UpdateSolution_hpp
