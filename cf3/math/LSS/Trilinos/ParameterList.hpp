// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_ParameterList_hpp
#define cf3_Math_LSS_ParameterList_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include "Teuchos_RCPDecl.hpp"

#include "common/Component.hpp"

#include "math/LSS/LibLSS.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  @file ParameterList.hpp Drives Trilinos linear solvers through Stratimikos
 *  @author Bart Janssens
 **/
////////////////////////////////////////////////////////////////////////////////////////////

namespace Teuchos { class ParameterList; }

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

/// Encapsulate a Teuchos parameter list
class LSS_API ParameterList : public common::Component
{
public:

  /// Default constructor
  ParameterList(const std::string& name);

  ~ParameterList();

  /// name of the type
  static std::string type_name () { return "ParameterList"; }

  /// Set the parameterlist, and add options for every parameter in the list.
  /// This method must only be called once.
  void set_parameter_list(Teuchos::ParameterList& parameters);

  /// Create a new ParameterList as a child to the current list
  /// @param trilinos_name The name of the parameter list as Trilinos expects it
  Handle<ParameterList> create_parameter_list(const std::string& trilinos_name);

  /// Create a new ParameterList as a child to the current list, reading settings from the XML file
  /// @param xmlpath XML file to read from
  void read_parameter_list(const std::string& xmlpath);
  
  /// Called when one of the parameter values changed, to update the underlying parameterlist
  void trigger_parameter_changed();

  /// Signal to add a parameter to the list
  void signal_add_parameter(common::SignalArgs& args);
  
  /// Signal and signature to create a new parameterlist
  void signal_create_parameter_list(common::SignalArgs& args);
  void signature_create_parameter_list(common::SignalArgs& args);
  void signal_read_parameter_list(common::SignalArgs& args);
  void signature_read_parameter_list(common::SignalArgs& args);

private:
  Teuchos::RCP<Teuchos::ParameterList> m_parameters;

}; // end of class ParameterList

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_ParameterList_hpp
