// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_RoeFluxSplitterCons1D_hpp
#define CF_FVM_RoeFluxSplitterCons1D_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Math/MatrixTypes.hpp"
#include "FVM/LibFVM.hpp"

namespace CF {
namespace FVM {


////////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
class FVM_API RoeFluxSplitterCons1D : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<RoeFluxSplitterCons1D> Ptr;
  typedef boost::shared_ptr<RoeFluxSplitterCons1D const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  RoeFluxSplitterCons1D ( const std::string& name );

  /// Virtual destructor
  virtual ~RoeFluxSplitterCons1D();

  /// Get the class name
  static std::string type_name () { return "RoeFluxSplitterCons1D"; }

  // functions specific to the RoeFluxSplitterCons1D component
  RealVector interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal);

  void solve(const RealVector& left, const RealVector& right, const RealVector& normal, 
             RealVector& flux, Real& left_wave_speed, Real& right_wave_speed);

  void compute_roe_average(const RealVector& left, const RealVector& right, RealVector& roe_avg) const;
  
  RealVector flux(const RealVector& state) const;
    
private:
  
  /// gamma
  const Real m_g;
  
  /// gamma - 1 
  const Real m_gm1;
  
  RealVector m_roe_avg;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_RoeFluxSplitterCons1D_hpp
