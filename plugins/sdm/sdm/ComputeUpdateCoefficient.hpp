// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ComputeUpdateCoefficient_hpp
#define cf3_sdm_ComputeUpdateCoefficient_hpp

#include "solver/Action.hpp"
#include "sdm/LibSDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; }
namespace solver { class Time;   }
namespace sdm {


class sdm_API ComputeUpdateCoefficient : public solver::Action
{
public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeUpdateCoefficient ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeUpdateCoefficient() {};

  /// Get the class name
  static std::string type_name () { return "ComputeUpdateCoefficient"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  Real limit_end_time(const Real& time, const Real& end_time);
  void link_fields();
private: // data

  Handle<mesh::Field> m_update_coeff;
  Handle<mesh::Field> m_wave_speed;
  Handle<solver::Time> m_time;

  Real m_tolerance;
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_ComputeUpdateCoefficient_hpp
