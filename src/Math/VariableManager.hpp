// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_VariableManager_hpp
#define CF_Math_VariableManager_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Math/LibMath.hpp"

namespace CF {
namespace Math {

class VariablesDescriptor;

////////////////////////////////////////////////////////////////////////////////

/// Manage the variables needed in a model
/// @author Bart Janssens
/// @author Tiago Quintino
class Math_API VariableManager : public Common::Component {

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

  /// Create a VariablesDescriptor with the given variables string
  VariablesDescriptor& create_descriptor(const std:string& descriptor);
  
  /// @name SIGNALS
  //@{
    
  void signal_create_descriptor(Common::SignalArgs& node);
    
  //@} END SIGNALS
                        
}; // VariableManager

////////////////////////////////////////////////////////////////////////////////

} // Math
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_VariableManager_hpp
