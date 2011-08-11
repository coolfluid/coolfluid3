// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_RK_hpp
#define CF_RDM_RK_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh { class CField; }
namespace RDM {

/// Runge-Kutta update step
/// @author Tiago Quintino
class RDM_API RK : public CF::Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<RK> Ptr;
  typedef boost::shared_ptr<RK const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  RK ( const std::string& name );

  /// Virtual destructor
  virtual ~RK() {}

  /// Get the class name
  static std::string type_name () { return "RK"; }

  /// execute the action
  virtual void execute ();

private: // data

  /// solution field pointer
  boost::weak_ptr<Mesh::CField> m_solution;
  /// residual field pointer
  boost::weak_ptr<Mesh::CField> m_residual;
  /// dual_area field pointer
  boost::weak_ptr<Mesh::CField> m_dual_area;

};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_RK_hpp
