// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RiemannSolvers_Roe_hpp
#define CF_RiemannSolvers_Roe_hpp

////////////////////////////////////////////////////////////////////////////////

#include "RiemannSolvers/RiemannSolver.hpp"
#include "Solver/Physics.hpp"

namespace CF {
namespace RiemannSolvers {

////////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
class RiemannSolvers_API Roe : public RiemannSolver {

public: // typedefs

  typedef boost::shared_ptr<Roe> Ptr;
  typedef boost::shared_ptr<Roe const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Roe ( const std::string& name );

  /// Virtual destructor
  virtual ~Roe();

  /// Get the class name
  static std::string type_name () { return "Roe"; }

  virtual void solve(const RealVector& left, const RealVector& right, const RealVector& normal, 
             RealVector& flux, Real& left_wave_speed, Real& right_wave_speed);
  
  void setup();

private:

  void build_roe_state();
  
  boost::shared_ptr<Solver::State> m_roe_state;

  Solver::Physics m_roe_avg_vars;
  std::vector<Solver::Physics> m_phys_vars;

  RealMatrix right_eigenvectors;

  RealMatrix left_eigenvectors;

  RealVector eigenvalues;

  RealMatrix abs_jacobian;

  RealVector F_L;

  RealVector F_R;
};

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RiemannSolvers_Roe_hpp
