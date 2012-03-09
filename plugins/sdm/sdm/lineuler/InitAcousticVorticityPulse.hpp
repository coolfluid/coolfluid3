// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_InitAcousticVorticityPulse_hpp
#define cf3_sdm_lineuler_InitAcousticVorticityPulse_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"

#include "math/MatrixTypes.hpp"
#include "math/Integrate.hpp"

#include "sdm/lineuler/LibLinEuler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { class Field; }
namespace sdm { 
namespace lineuler {

//////////////////////////////////////////////////////////////////////////////

/// This configurable function creates an initial condition for a
/// linearized Euler benchmark case
/// @author Willem Deconinck
class  InitAcousticVorticityPulse : public common::Action
{
public: // functions
  
  /// constructor
  InitAcousticVorticityPulse( const std::string& name );
  
  virtual ~InitAcousticVorticityPulse() {}
  /// Gets the Class name
  static std::string type_name() { return "InitAcousticVorticityPulse"; }

  virtual void execute();
  
  RealVector compute_velocity(const RealVector& coord, const Real& t);
  Real compute_pressure(const RealVector& coord, const Real& t);
  Real compute_density(const Real& pressure, const RealVector& coord, const Real& t);

  Real eta(const RealVector& coord, const Real& t) const;

  struct Data
  {
    Data();
    Real u0;            //!< advection speed
    Real alpha1;        //!< coefficient
    Real alpha2;        //!< coefficient
    Real eta;
    Real time;

    Real s0;  //!< integration lower bound
    Real s1;  //!< integration upper bound
  };

  class PressureIntegrand
  {
  public:
    PressureIntegrand(const InitAcousticVorticityPulse::Data& data) :
      m_data(data)
    {}

    /// Actual function to be integrated
    Real operator()(Real lambda) const;

  private:
    const InitAcousticVorticityPulse::Data& m_data;
  };

  class VelocityIntegrand
  {
  public:
    VelocityIntegrand(const InitAcousticVorticityPulse::Data& data) :
      m_data(data)
    {}

    /// Actual function to be integrated
    Real operator()(Real lambda) const;

  private:
    const InitAcousticVorticityPulse::Data& m_data;
  };


private: // data

  Handle<mesh::Field> m_field;
  Data m_data;
  math::Integrate integrate;

}; // end InitAcousticVorticityPulse


////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_InitAcousticVorticityPulse_hpp
