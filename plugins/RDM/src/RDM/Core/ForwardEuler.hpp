// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_ForwardEuler_hpp
#define CF_RDM_ForwardEuler_hpp

#include "Solver/Action.hpp"

#include "RDM/Core/LibCore.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh { class CField; }
namespace RDM {

class RDM_Core_API ForwardEuler : public CF::Solver::Action
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<ForwardEuler> Ptr;
  typedef boost::shared_ptr<ForwardEuler const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  ForwardEuler ( const std::string& name );

  /// Virtual destructor
  virtual ~ForwardEuler() {};

  /// Get the class name
  static std::string type_name () { return "ForwardEuler"; }
  
  /// execute the action
  virtual void execute ();

protected: // helper functions

  bool stop_condition();

private: // data

  /// CFL number
  CF::Real m_cfl;
  /// maximum number of iterations
  Uint m_max_iter;

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

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_ForwardEuler_hpp
