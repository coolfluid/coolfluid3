// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_DynamicVars_hpp
#define cf3_physics_DynamicVars_hpp

#include "physics/DynamicModel.hpp"
#include "physics/Variables.hpp"

namespace cf3 {
namespace physics {

///////////////////////////////////////////////////////////////////////////////////////

class physics_API DynamicVars : public Variables {

public: // functions

  typedef DynamicModel     MODEL;

  
  

public: // functions

  /// constructor
  /// @param name of the component
  DynamicVars ( const std::string& name );

  /// virtual destructor
  virtual ~DynamicVars();

  /// Get the class name
  static std::string type_name () { return "DynamicVars"; }

  /// @name INTERFACE
  //@{

  /// @return the variables type
  virtual std::string type() const { return DynamicVars::type_name(); };

  /// compute physical properties
  virtual void compute_properties (const RealVector& coord,
                                   const RealVector& sol,
                                   const RealMatrix& grad_sol,
                                   physics::Properties& physp)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute variables from properties
  virtual void compute_variables (const physics::Properties& physp,
                                  RealVector& vars)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute the physical flux
  virtual void flux (const physics::Properties& p,
                     RealMatrix& flux)
  {
    /// @todo to be implemented in the .cpp
  }

  virtual void flux (const physics::Properties& p,
                     const RealVector& direction,
                     RealVector& flux)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute the eigen values of the flux jacobians
  virtual void flux_jacobian_eigen_values (const physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute the eigen values of the flux jacobians
  /// and apply a provided operator
  virtual void flux_jacobian_eigen_values (const physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues,
                                           UnaryRealOp& op )
  {
    /// @todo to be implemented in the .cpp
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  virtual void flux_jacobian_eigen_structure (const physics::Properties& p,
                                              const RealVector& direction,
                                              RealMatrix& Rv,
                                              RealMatrix& Lv,
                                              RealVector& evalues)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute the PDE residual
  virtual void residual(const physics::Properties& p,
                        RealMatrix  flux_jacob[],
                        RealVector& res)
  {
    /// @todo to be implemented in the .cpp
  }

  virtual math::VariablesDescriptor& description()
  {
    throw common::NotSupported(FromHere(),"querying description not supported for DynamicVars, see VariableManager");
    static boost::shared_ptr< math::VariablesDescriptor > desc (common::allocate_component<math::VariablesDescriptor>("desc"));
    return *desc;
  }
  //@} END INTERFACE

}; // DynamicVars

////////////////////////////////////////////////////////////////////////////////////

} // physics
} // cf3

#endif // cf3_physics_DynamicVars_hpp
