// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ComputeCFL_hpp
#define cf3_UFEM_ComputeCFL_hpp

#include "solver/actions/Proto/ProtoAction.hpp"


#include "LibUFEM.hpp"

namespace cf3 {
  namespace solver { class Time; }
namespace UFEM {

/// Boundary condition to hold the value of a field at a value given by another (or the same) field
class UFEM_API ComputeCFL : public solver::actions::Proto::ProtoAction
{
public:
  /// Contructor
  /// @param name of the component
  ComputeCFL ( const std::string& name );

  virtual ~ComputeCFL();

  /// Get the class name
  static std::string type_name () { return "ComputeCFL"; }

  virtual void execute();

private:
  /// Trigger executed when the tag or the variable name are changed
  void trigger_variable();
  Real m_cfl_scaling;
  Real m_max_cfl;
  Real m_dt;
  Real m_min_dt;
  Handle<solver::Time> m_time;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_ComputeCFL_hpp
