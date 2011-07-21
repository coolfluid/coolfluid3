// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_FwdEuler_hpp
#define CF_RDM_FwdEuler_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh { class CField; }
namespace RDM {

class RDM_API FwdEuler : public CF::Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<FwdEuler> Ptr;
  typedef boost::shared_ptr<FwdEuler const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  FwdEuler ( const std::string& name );

  /// Virtual destructor
  virtual ~FwdEuler() {}

  /// Get the class name
  static std::string type_name () { return "FwdEuler"; }

  /// execute the action
  virtual void execute ();

private: // data

  /// solution field pointer
  boost::weak_ptr<Mesh::CField> m_solution;
  /// residual field pointer
  boost::weak_ptr<Mesh::CField> m_residual;
  /// wave_speed field pointer
  boost::weak_ptr<Mesh::CField> m_wave_speed;

};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_FwdEuler_hpp
