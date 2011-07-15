// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_Variables_hpp
#define CF_Physics_Variables_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "Common/Component.hpp"

#include "Math/MatrixTypes.hpp"

#include "Physics/PhysModel.hpp"

namespace CF {

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
class Physics_API Variables : public Common::Component {

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
                                   const RealVector& sol,
                                   const RealMatrix& grad_sol,
                                   Physics::Properties& physp) = 0;

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


  //@} END INTERFACE
                        
  /// @name Variable management
  /// Generic functionality to manage the variables that are used in a solver, including options to control field and variable names
  //@{
  
  /// Available variable types
  enum VariableTypesT
  {
    SCALAR = 0,
    VECTOR = 1
  };
  
  /// Register a variable. The order of registration also determines the storage order for the equations in the physical model.
  /// The symbol and field_name parameters are linked to options that allow user control.If a variable with the same
  /// name was already registered, nothing is changed, except for the option linking.
  /// @param name Unique name by which this value is referred (internal to the model).
  /// @param symbol Short name for the variable.  By default, the variable will be named like this in the field
  /// The given string is also linked to an option that gets created, allowing the user to change the name of this variable
  /// @param field_name Default field name
  /// The given string is also linked to an option that gets created, allowing the user to change the name of the field
  /// @param var_type Type of the variable
  /// @param is_equation_var True if the variable represents a state, i.e. something that is solved for
  void register_variable(const std::string& var_name, std::string& symbol, std::string& field_name, const VariableTypesT var_type, const bool is_state);
  
  /// True if the variable with the given name is part of the solution state
  bool is_state_variable(const std::string& var_name) const;
  
  /// Get the offset in the state for the variable with the supplied name, i.e. if the variables are ordered u, v, p in the system, the offset for p is 2.
  Uint offset(const std::string& var_name) const;
  
  /// @return the number of degrees of freedom (DOFs), i.e. the number of components of the state vector (the number of scalars needed to represent
  /// the solution at a single node)
  Uint nb_dof() const;
  
  /// Return the type of the variable with the given key
  VariableTypesT variable_type(const std::string& var_name) const;
  
  /// Stores the names of fields used for state variables in fieldlist.
  void state_fields(std::vector< std::string >& fieldlist) const;
  
  /// Store the fields and their varible in the supplied map
  /// @param fields map to fill, where the key will be the field name and the value the string specifying the variables of the field,
  /// using the CField protocol
  void field_specification(std::map<std::string, std::string>& fields);
  
  //@} End Variable management

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
                        
}; // Variables

////////////////////////////////////////////////////////////////////////////////

/// Template class that provides a dyanmic wrapper around a static implemented
/// variables class
/// @author Tiago Quintino
template < typename PHYS >
class VariablesT : public Variables {
public:

  /// constructor
  VariablesT ( const std::string& name ) : Variables( name )
  {
    regist_typeinfo(this);
  }

  /// virtual destructor
  virtual ~VariablesT() {}

  /// Get the class name
  static std::string type_name () { return "VariablesT<"+PHYS::type_name()+">"; }

  /// @return the physical model type
  virtual std::string type() const { return PHYS::type_name(); };

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

}; // VariablesT

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Physics_Variables_hpp
