// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_TimeIntegrationStepComputer_hpp
#define cf3_sdm_TimeIntegrationStepComputer_hpp

#include "common/Action.hpp"
#include "sdm/LibSDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; }
namespace solver { class Time;   }
namespace sdm {


class sdm_API TimeIntegrationStepComputer : public common::Action
{
public: // functions
  /// Contructor
  /// @param name of the component
  TimeIntegrationStepComputer ( const std::string& name );

  /// Virtual destructor
  virtual ~TimeIntegrationStepComputer() {};

  /// Get the class name
  static std::string type_name () { return "TimeIntegrationStepComputer"; }

protected: // data

  Handle<mesh::Field> m_update_coeff;
  Handle<mesh::Field> m_wave_speed;
  Handle<solver::Time> m_time;

};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_TimeIntegrationStepComputer_hpp
