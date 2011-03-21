// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_RoeCons2D_hpp
#define CF_FVM_RoeCons2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "FVM/RiemannSolver.hpp"

namespace CF {
namespace FVM {


////////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
class FVM_API RoeCons2D : public FVM::RiemannSolver {

public: // typedefs

  typedef boost::shared_ptr<RoeCons2D> Ptr;
  typedef boost::shared_ptr<RoeCons2D const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  RoeCons2D ( const std::string& name );

  /// Virtual destructor
  virtual ~RoeCons2D();

  /// Get the class name
  static std::string type_name () { return "RoeCons2D"; }

  // functions specific to the RoeCons2D component
  virtual void solve(const RealVector& left, const RealVector& right, const RealVector& normal, 
             RealVector& flux, Real& left_wave_speed, Real& right_wave_speed);
  
  virtual RealVector flux(const RealVector& state, const RealVector& normal) const;
  
  void compute_flux(const RealVector& state, const RealVector& normal, RealVector4& flux) const;

  void compute_roe_average(const RealVector& left, const RealVector& right, RealVector& roe_avg) const;
    
private:
  
  RealVector m_roe_avg;
  
  RealMatrix4 right_eigenvectors; 
  
  RealMatrix4 left_eigenvectors; 

  RealVector4 eigenvalues; 

  RealMatrix4 abs_jacobian;
  
  RealVector4 F_L;
  RealVector4 F_R;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_RoeCons2D_hpp
