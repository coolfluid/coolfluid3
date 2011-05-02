// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_RiemannSolver_hpp
#define CF_FVM_Core_RiemannSolver_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Math/MatrixTypes.hpp"
#include "FVM/Core/LibCore.hpp"

namespace CF {
namespace FVM {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
class FVM_Core_API RiemannSolver : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<RiemannSolver> Ptr;
  typedef boost::shared_ptr<RiemannSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  RiemannSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~RiemannSolver();

  /// Get the class name
  static std::string type_name () { return "RiemannSolver"; }

  // functions specific to the RiemannSolver component
  RealVector interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal);

  virtual void solve(const RealVector& left, const RealVector& right, const RealVector& normal, 
             RealVector& flux, Real& left_wave_speed, Real& right_wave_speed) = 0;
  
  virtual RealVector flux(const RealVector& state, const RealVector& normal) const = 0;
    
protected:
  
  /// gamma
  const Real m_g;
  
  /// gamma - 1 
  const Real m_gm1;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_RiemannSolver_hpp
