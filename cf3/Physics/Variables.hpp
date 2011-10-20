// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Physics_Variables_hpp
#define cf3_Physics_Variables_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "common/Component.hpp"

#include "Math/VariablesDescriptor.hpp"
#include "Math/MatrixTypes.hpp"

#include "Physics/PhysModel.hpp"

namespace cf3 {

namespace Physics {

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
class Physics_API Variables : public common::Component {

public: //typedefs

  typedef boost::shared_ptr<Variables> Ptr;
  typedef boost::shared_ptr<Variables const> ConstPtr;

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
                                   Physics::Properties& physp) = 0;

  /// compute variables from properties
  virtual void compute_variables (const Physics::Properties& physp,
                                  RealVector& vars) = 0;

  /// compute the physical flux
  virtual void flux (const Physics::Properties& p,
                     RealMatrix& flux) = 0;

  /// compute the eigen values of the flux jacobians
  virtual void flux_jacobian_eigen_values (const Physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues) = 0;

  /// compute the eigen values of the flux jacobians
  /// and apply a provided operator
  virtual void flux_jacobian_eigen_values (const Physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues,
                                           UnaryRealOp& op ) = 0;

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  virtual void flux_jacobian_eigen_structure (const Physics::Properties& p,
                                              const RealVector& direction,
                                              RealMatrix& Rv,
                                              RealMatrix& Lv,
                                              RealVector& evalues) = 0;
  /// compute the PDE residual
  virtual void residual(const Physics::Properties& p,
                        RealMatrix  flux_jacob[],
                        RealVector& res) = 0;


  virtual Math::VariablesDescriptor& description() = 0;

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
    m_description (common::allocate_component<Math::VariablesDescriptor>("description"))
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
                                  Physics::Properties& p)
  {
    typename PHYS::MODEL::Properties& cp =
        static_cast<typename PHYS::MODEL::Properties&>( p );

    PHYS::compute_properties( coord, sol, grad_sol, cp );
  }

  virtual void compute_variables (const Physics::Properties& p,
                                  RealVector& vars)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::compute_variables( cp, vars );
  }

  /// compute the physical flux
  virtual void flux (const Physics::Properties& p,
                     RealMatrix& flux)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::flux( cp, flux );
  }

  /// compute the eigen values of the flux jacobians
  virtual void flux_jacobian_eigen_values (const Physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::flux_jacobian_eigen_values( cp, direction, evalues );
  }

  /// compute the eigen values of the flux jacobians
  /// and apply a provided operator
  virtual void flux_jacobian_eigen_values (const Physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues,
                                           UnaryRealOp& op )
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::flux_jacobian_eigen_values( cp, direction, evalues, op );
  }


  /// decompose the eigen structure of the flux jacobians projected on the gradients
  virtual void flux_jacobian_eigen_structure (const Physics::Properties& p,
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
  virtual void residual(const Physics::Properties& p,
                        RealMatrix  flux_jacob[],
                        RealVector& res)
  {
    typename PHYS::MODEL::Properties const& cp =
        static_cast<typename PHYS::MODEL::Properties const&>( p );

    PHYS::residual( cp, flux_jacob, res );
  }

  virtual Math::VariablesDescriptor& description() { return *m_description; }

private:
  boost::shared_ptr<Math::VariablesDescriptor> m_description;

}; // VariablesT

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Physics_Variables_hpp
