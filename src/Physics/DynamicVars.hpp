// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Physics_DynamicVars_hpp
#define cf3_Physics_DynamicVars_hpp

#include "Physics/DynamicModel.hpp"
#include "Physics/Variables.hpp"

namespace cf3 {
namespace Physics {

///////////////////////////////////////////////////////////////////////////////////////

class Physics_API DynamicVars : public Variables {

public: // functions

  typedef DynamicModel     MODEL;

  typedef boost::shared_ptr<DynamicVars> Ptr;
  typedef boost::shared_ptr<DynamicVars const> ConstPtr;

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
                                   Physics::Properties& physp)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute variables from properties
  virtual void compute_variables (const Physics::Properties& physp,
                                  RealVector& vars)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute the physical flux
  virtual void flux (const Physics::Properties& p,
                     RealMatrix& flux)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute the eigen values of the flux jacobians
  virtual void flux_jacobian_eigen_values (const Physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute the eigen values of the flux jacobians
  /// and apply a provided operator
  virtual void flux_jacobian_eigen_values (const Physics::Properties& p,
                                           const RealVector& direction,
                                           RealVector& evalues,
                                           UnaryRealOp& op )
  {
    /// @todo to be implemented in the .cpp
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  virtual void flux_jacobian_eigen_structure (const Physics::Properties& p,
                                              const RealVector& direction,
                                              RealMatrix& Rv,
                                              RealMatrix& Lv,
                                              RealVector& evalues)
  {
    /// @todo to be implemented in the .cpp
  }

  /// compute the PDE residual
  virtual void residual(const Physics::Properties& p,
                        RealMatrix  flux_jacob[],
                        RealVector& res)
  {
    /// @todo to be implemented in the .cpp
  }

  virtual Math::VariablesDescriptor& description()
  {
    throw common::NotSupported(FromHere(),"querying description not supported for DynamicVars, see VariableManager");
    static Math::VariablesDescriptor::Ptr desc (common::allocate_component<Math::VariablesDescriptor>("desc"));
    return *desc;
  }
  //@} END INTERFACE

}; // DynamicVars

////////////////////////////////////////////////////////////////////////////////////

} // Physics
} // cf3

#endif // CF3_Physics_DynamicVars_hpp
