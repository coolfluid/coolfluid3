// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_PhysModel_hpp
#define cf3_physics_PhysModel_hpp

#include "common/Component.hpp"

#include "physics/LibPhysics.hpp"

namespace cf3 {
namespace math { class VariableManager; }
namespace physics {

  class Variables; // forward declaration

  /// base type for the physical properties
  /// @note class is non copyable becase it might contain Eigen matrices
  /// @note this class and its derived classes should not ave any virtual functions
  struct Properties : public boost::noncopyable {};

////////////////////////////////////////////////////////////////////////////////

/// Component providing information about the physics
/// @author Tiago Quintino
/// @author Willem Deconinck
/// @author Bart Janssens
class physics_API PhysModel : public common::Component {

public: //typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  PhysModel ( const std::string& name );

  /// Virtual destructor
  virtual ~PhysModel();

  /// Get the class name
  static std::string type_name () { return "PhysModel"; }


  void signal_create_variables( common::SignalArgs& args );
  void signature_create_variables( common::SignalArgs& args );

  /// @name INTERFACE
  //@{

  /// @return dimensionality of the problem, which is
  ///         the number of spatial coordinates used in the PDEs
  virtual Uint ndim() const = 0;

  /// @return number of independent variables in the PDE
  virtual Uint neqs() const = 0;

  /// @return the physical model generic type
  virtual std::string model_type() const = 0;
  /// @return the physical model type
  virtual std::string type() const = 0;

  /// create a physical properties
  virtual std::auto_ptr< physics::Properties > create_properties() = 0;

  /// Create a Variables component
  /// @param type is the name of the Variables
  /// @post the component will be a sub-component of this model but maybe be moved away
  /// @throws ValueNotFound if the type does not match a variable type this model supports
  virtual boost::shared_ptr< physics::Variables > create_variables( const std::string type, const std::string name ) = 0;

  //@} END INTERFACE

  /// Access to the VariableManager
  math::VariableManager& variable_manager();
  const math::VariableManager& variable_manager() const;

private:
  math::VariableManager& m_variable_manager;

}; // PhysModel

////////////////////////////////////////////////////////////////////////////////


} // physics
} // cf3

#endif // cf3_physics_PhysModel_hpp
