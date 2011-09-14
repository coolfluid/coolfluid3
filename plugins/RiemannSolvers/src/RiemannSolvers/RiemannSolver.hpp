// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RiemannSolvers_RiemannSolver_hpp
#define CF_RiemannSolvers_RiemannSolver_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Math/MatrixTypes.hpp"
#include "RiemannSolvers/src/RiemannSolvers/LibRiemannSolvers.hpp"

namespace CF {
namespace Physics { class Variables; }
namespace RiemannSolvers {

////////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
class RiemannSolvers_API RiemannSolver : public Common::Component
{
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
  virtual void compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal,
                                      RealVector& flux, Real& wave_speed) = 0;

  const Physics::Variables& sol_vars() const { return *m_solution_vars.lock(); }

protected:
  boost::weak_ptr<Physics::Variables> m_solution_vars;
};

////////////////////////////////////////////////////////////////////////////////

template <class T>
struct extract_var
{
};

template <template <class> class Solver, class Var>
struct extract_var< Solver<Var> >
{
  typedef Var type;
};

template <template <class,class> class Solver, class Var, class Avg>
struct extract_var< Solver<Var,Avg> >
{
  typedef Var type;
};

template <typename RIEMANNSOLVER>
class RiemannSolverT : public RiemannSolver
{
public:
  typedef typename extract_var<RIEMANNSOLVER>::type VAR;
//  typedef Eigen::Matrix<Real, VAR::MODEL::_neqs, 1> FluxT;
//  typedef Eigen::Matrix<Real, VAR::MODEL::_ndim, 1> CoordsT;

  typedef RIEMANNSOLVER SOLVER;

public:
  /// Contructor
  /// @param name of the component
  RiemannSolverT ( const std::string& name ) : RiemannSolver(name) {}

  /// Virtual destructor
  virtual ~RiemannSolverT() {};

  virtual void compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal,
                                      RealVector& flux, Real& wave_speed)
  {
    SOLVER::compute_interface_flux(left,right,normal,flux,wave_speed);
  }

};

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RiemannSolvers_RiemannSolver_hpp
