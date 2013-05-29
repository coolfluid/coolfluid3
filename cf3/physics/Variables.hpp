// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_Variables_hpp
#define cf3_physics_Variables_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "common/Component.hpp"

#include "math/VariablesDescriptor.hpp"
#include "math/MatrixTypes.hpp"

#include "physics/PhysModel.hpp"

namespace cf3 {

namespace physics {

  class Variables;

  /// base unary real operation
  /// deriving from std::unary_function is not necessary but provides type knowledge
  struct UnaryRealOp : public std::unary_function<Real,Real>
  {
    virtual Real operator() ( const Real& r ) const { return r; };
  };

////////////////////////////////////////////////////////////////////////////////

/// Interface to a set of variables
/// @author Tiago Quintino
class physics_API Variables : public common::Component {

public: //typedefs

  
  

public: // functions

  /// constructor
  /// @param name of the component
  Variables ( const std::string& name );

  /// virtual destructor
  virtual ~Variables();

  /// Get the class name
  static std::string type_name () { return "Variables"; }

  /// @name INTERFACE
  //@{

  /// @return the variables type
  virtual std::string type() const = 0;

  /// compute physical properties
  virtual void compute_properties (const RealVector& coord,
                                   const RealVector& vars,
                                   const RealMatrix& grad_vars,
                                   physics::Properties& physp) = 0;

  /// compute variables from properties
  virtual void compute_variables (const physics::Properties& physp,
                                  RealVector& vars) = 0;

  /// compute the physical flux
  virtual void flux (const physics::Properties& p,
                     RealMatrix& flux) = 0;

  /// compute the physical flux
  virtual void flux (const physics::Properties& p,
                     const RealVector& direction,
                     RealVector& flux) = 0;

  /// compute the eigen values of the flux jacobians
  virtual void flux_jacobian_eigen_values (const physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues) = 0;

  /// compute the eigen values of the flux jacobians
  /// and apply a provided operator
  virtual void flux_jacobian_eigen_values (const physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues,
                                           UnaryRealOp& op ) = 0;

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  virtual void flux_jacobian_eigen_structure (const physics::Properties& p,
                                              const RealVector& direction,
                                              RealMatrix& Rv,
                                              RealMatrix& Lv,
                                              RealVector& evalues) = 0;
  /// compute the PDE residual
  virtual void residual(const physics::Properties& p,
                        RealMatrix  flux_jacob[],
                        RealVector& res) = 0;


  virtual math::VariablesDescriptor& description() = 0;

  //@} END INTERFACE

}; // Variables

////////////////////////////////////////////////////////////////////////////////

/// Template class that provides a dynamic wrapper around a static implemented
/// variables class
/// @author Tiago Quintino
template < typename PHYS >
class VariablesT : public Variables {
public:

  /// constructor
  VariablesT ( const std::string& name ) :
    Variables( name ),
    m_description (common::allocate_component<math::VariablesDescriptor>("description"))
  {
    regist_typeinfo(this);
    add_static_component (m_description);
  }

  /// virtual destructor
  virtual ~VariablesT() {}

  /// Get the class name
  static std::string type_name () { return "VariablesT<"+PHYS::type_name()+">"; }

  /// @return the physical model type
  virtual std::string type() const { return PHYS::type_name(); }

  /// compute physical properties
  virtual void compute_properties(const RealVector& coord,
                                  const RealVector& sol,
                                  const RealMatrix& grad_sol,
                                  physics::Properties& p)
  {
    typename PHYS::MODEL::Properties& cp =
        static_cast<typename PHYS::MODEL::Properties&>( p );

    PHYS::compute_properties( coord, sol, grad_sol, cp );
  }

  virtual void compute_variables (const physics::Properties& p,
                                  RealVector& vars)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::compute_variables( cp, vars );
  }

  /// compute the physical flux
  virtual void flux (const physics::Properties& p,
                     RealMatrix& flux)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::flux( cp, flux );
  }

  /// compute the physical flux in a direction
  virtual void flux (const physics::Properties& p,
                     const RealVector& direction,
                     RealVector& flux)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::flux( cp, direction, flux );
  }

  /// compute the eigen values of the flux jacobians
  virtual void flux_jacobian_eigen_values (const physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::flux_jacobian_eigen_values( cp, direction, evalues );
  }

  /// compute the eigen values of the flux jacobians
  /// and apply a provided operator
  virtual void flux_jacobian_eigen_values (const physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues,
                                           UnaryRealOp& op )
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::flux_jacobian_eigen_values( cp, direction, evalues, op );
  }


  /// decompose the eigen structure of the flux jacobians projected on the gradients
  virtual void flux_jacobian_eigen_structure (const physics::Properties& p,
                                              const RealVector& direction,
                                              RealMatrix& Rv,
                                              RealMatrix& Lv,
                                              RealVector& evalues)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::flux_jacobian_eigen_structure( cp, direction, Rv, Lv, evalues );

  }

  /// compute the PDE residual
  virtual void residual(const physics::Properties& p,
                        RealMatrix  flux_jacob[],
                        RealVector& res)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::residual( cp, flux_jacob, res );
  }

  virtual math::VariablesDescriptor& description() { return *m_description; }

private:
  boost::shared_ptr<math::VariablesDescriptor> m_description;

}; // VariablesT

////////////////////////////////////////////////////////////////////////////////

} // physics
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_physics_Variables_hpp
