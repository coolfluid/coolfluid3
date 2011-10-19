// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_VariableManager_hpp
#define cf3_Math_VariableManager_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Math/LibMath.hpp"

namespace cf3 {
namespace Math {

class VariablesDescriptor;

////////////////////////////////////////////////////////////////////////////////

/// Manage the variables needed in a model
/// @author Bart Janssens
/// @author Tiago Quintino
class Math_API VariableManager : public common::Component {

public: //typedefs

  typedef boost::shared_ptr<VariableManager> Ptr;
  typedef boost::shared_ptr<VariableManager const> ConstPtr;

public: // functions

  /// constructor
  /// @param name of the component
  VariableManager ( const std::string& name );

  /// virtual destructor
  virtual ~VariableManager();

  /// Get the class name
  static std::string type_name () { return "VariableManager"; }

  /// Create a VariablesDescriptor
  /// @param name Name of the VariablesDescriptor component that gets created. The name is also automatically added as a tag.
  /// @param description String that describes the variables in the created descriptor
  VariablesDescriptor& create_descriptor(const std::string& name, const std::string& description);

  /// @name SIGNALS
  //@{

  void signal_create_descriptor(common::SignalArgs& node);

  //@} END SIGNALS

private:
  void signature_create_descriptor(common::SignalArgs& node);
}; // VariableManager

////////////////////////////////////////////////////////////////////////////////

} // Math
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Math_VariableManager_hpp
