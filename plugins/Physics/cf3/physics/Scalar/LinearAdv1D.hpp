// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_Scalar_LinearAdv1D_hpp
#define cf3_physics_Scalar_LinearAdv1D_hpp

#include "cf3/common/StringConversion.hpp"
#include "math/Defs.hpp"

#include "cf3/physics/Variables.hpp"

#include "cf3/physics/Scalar/Scalar1D.hpp"

namespace cf3 {
namespace physics {
namespace Scalar {

///////////////////////////////////////////////////////////////////////////////////////

class Scalar_API LinearAdv1D : public VariablesT<LinearAdv1D> {

public: //typedefs

  typedef Scalar1D     MODEL;

  enum { U = 0 };

  
  

public: // functions

  /// constructor
  /// @param name of the component
  LinearAdv1D ( const std::string& name );

  /// virtual destructor
  virtual ~LinearAdv1D();

  /// Get the class name
  static std::string type_name () { return "LinearAdv1D"; }

  /// compute physical properties
  template < typename CV, typename SV, typename GM >
  static void compute_properties ( const CV& coord,
                                   const SV& sol,
                                   const GM& grad_vars,
                                   MODEL::Properties& p )
  {
    cf3_assert(coord.size()==MODEL::_ndim);
    cf3_assert(sol.size()==MODEL::_neqs);
    cf3_assert(grad_vars.rows()==MODEL::_ndim);
    cf3_assert(grad_vars.cols()==MODEL::_neqs);
    p.coords    = coord;       // cache the coordiantes locally
    p.vars      = sol;         // cache the variables locally
    p.grad_vars = grad_vars;   // cache the gradient of variables locally

    p.u = sol[U];
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[U]  = p.u;
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    flux(0,XX)   = p.v * p.u;
  }

  /// compute the physical flux
  template < typename FM , typename GV>
  static void flux( const MODEL::Properties& p,
                    const GV& direction,
                    FM& flux)
  {
    flux[0] = p.u * p.v * direction[XX];
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    Dv[0]   = p.v * direction[XX];
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    Dv[0]   = op( p.v * direction[XX] );
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename GV, typename EM, typename EV >
  static void flux_jacobian_eigen_structure(const MODEL::Properties& p,
                                            const GV& direction,
                                            EM& Rv,
                                            EM& Lv,
                                            EV& Dv)
  {
    Rv(0,0) = 1.;
    Lv(0,0) = 1.;
    Dv[0]   = p.v * direction[XX];
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    JM& A = flux_jacob[XX];

    A(0,0) = p.v;

    res = A * p.grad_vars.col(XX);
  }

}; // LinearAdv1D

////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // physics
} // cf3

#endif // cf3_physics_Scalar_LinearAdv1D_hpp
