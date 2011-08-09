// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_ComputeUpdateCoefficient_hpp
#define CF_SFDM_ComputeUpdateCoefficient_hpp

#include "Common/CAction.hpp"
#include "SFDM/LibSFDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh   { class CField; }
namespace Solver { class CTime;   }
namespace SFDM {


class SFDM_API ComputeUpdateCoefficient : public Common::CAction
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<ComputeUpdateCoefficient> Ptr;
  typedef boost::shared_ptr<ComputeUpdateCoefficient const> ConstPtr;

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

  void config_update_coeff();
  void config_wave_speed();
  void config_volume();
  void config_time();
  Real limit_end_time(const Real& time, const Real& end_time);

private: // data

  boost::weak_ptr<Mesh::CField> m_update_coeff;
  boost::weak_ptr<Mesh::CField> m_wave_speed;
  boost::weak_ptr<Mesh::CField> m_volume;
  boost::weak_ptr<Solver::CTime> m_time;

  bool m_time_accurate;
  Real m_CFL;

  bool m_freeze;

  Real m_tolerance;
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_ComputeUpdateCoefficient_hpp
