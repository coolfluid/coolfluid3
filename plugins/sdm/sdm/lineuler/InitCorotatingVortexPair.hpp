// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_InitCorotatingVortexPair_hpp
#define cf3_sdm_lineuler_InitCorotatingVortexPair_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"

#include "sdm/lineuler/LibLinEuler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { class Field; }
namespace solver { class Time; }
namespace sdm { 
namespace lineuler {

//////////////////////////////////////////////////////////////////////////////

/// This configurable function creates an initial condition for a
/// linearized Euler benchmark case
/// @author Willem Deconinck
class  InitCorotatingVortexPair : public common::Action
{
public: // functions
  
  /// constructor
  InitCorotatingVortexPair( const std::string& name );
  
  virtual ~InitCorotatingVortexPair() {}
  /// Gets the Class name
  static std::string type_name() { return "InitCorotatingVortexPair"; }

  virtual void execute();

private: // data

  Handle<solver::Time> m_time;
  Handle<mesh::Field> m_field;
  Handle<mesh::Field> m_mean_flow;
  Real m_omega;
  Real m_r0;
  Real m_gamma;

}; // end InitCorotatingVortexPair


////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_InitCorotatingVortexPair_hpp
