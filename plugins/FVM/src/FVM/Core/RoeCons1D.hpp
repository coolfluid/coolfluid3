// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_RoeCons1D_hpp
#define CF_FVM_Core_RoeCons1D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "FVM/Core/RiemannSolver.hpp"

namespace CF {
namespace FVM {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
class FVM_Core_API RoeCons1D : public RiemannSolver {

public: // typedefs

  typedef boost::shared_ptr<RoeCons1D> Ptr;
  typedef boost::shared_ptr<RoeCons1D const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  RoeCons1D ( const std::string& name );

  /// Virtual destructor
  virtual ~RoeCons1D();

  /// Get the class name
  static std::string type_name () { return "RoeCons1D"; }

  virtual void solve(const RealVector& left, const RealVector& right, const RealVector& normal, 
             RealVector& flux, Real& left_wave_speed, Real& right_wave_speed);
  
  virtual RealVector flux(const RealVector& state, const RealVector& normal) const;

  void compute_roe_average(const RealVector& left, const RealVector& right, RealVector& roe_avg) const;
    
private:
  
  RealVector m_roe_avg;
};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_RoeCons1D_hpp
