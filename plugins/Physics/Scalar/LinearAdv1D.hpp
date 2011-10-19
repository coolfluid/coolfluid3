// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Physics_Scalar_LinearAdv1D_hpp
#define cf3_Physics_Scalar_LinearAdv1D_hpp

#include "Common/StringConversion.hpp"
#include "Math/Defs.hpp"

#include "Physics/Variables.hpp"

#include "Scalar1D.hpp"

namespace cf3 {
namespace Physics {
namespace Scalar {

///////////////////////////////////////////////////////////////////////////////////////

class Scalar_API LinearAdv1D : public VariablesT<LinearAdv1D> {

public: //typedefs

  typedef Scalar1D     MODEL;

  enum { U = 0 };

  typedef boost::shared_ptr<LinearAdv1D> Ptr;
  typedef boost::shared_ptr<LinearAdv1D const> ConstPtr;

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
} // Physics
} // cf3

#endif // CF3_Physics_Scalar_LinearAdv1D_hpp
