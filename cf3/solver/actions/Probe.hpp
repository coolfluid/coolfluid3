// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Probe_hpp
#define cf3_solver_actions_Probe_hpp

////////////////////////////////////////////////////////////////////////////////


#include "common/Action.hpp"
#include "solver/actions/LibActions.hpp"

namespace cf3 {
namespace math { class VariablesDescriptor; }
namespace mesh { class Dictionary; class PointInterpolator; }
namespace solver {
namespace actions {

class ProbePostProcessor;

////////////////////////////////////////////////////////////////////////////////

/// @brief Probe to interpolate field values to a given coordinate
///
/// Interpolated values are stored as properties within the probe component.
/// Actions can be added as child to the probe, and will be executed, after
/// the probe is executed.
/// @author Willem Deconinck
class solver_actions_API Probe : public common::Action {
friend class ProbePostProcessor;
public: // functions

  /// Contructor
  /// @param name of the component
  Probe ( const std::string& name );

  /// Virtual destructor
  virtual ~Probe();

  /// Get the class name
  static std::string type_name () { return "Probe"; }

  virtual void execute();

  /// @brief Create a post processor action to be executed automatically after probing.
  Handle<ProbePostProcessor> create_post_processor(const std::string& name, const std::string& builder);

  /// @name SIGNALS
  //@{
  void signal_create_post_processor(common::SignalArgs& args);
  void signature_create_post_processor(common::SignalArgs& args);
  //@}

  /// @brief Access to the description of the probed variables
  Handle<math::VariablesDescriptor> variables() { return m_variables; }

private: // functions

  /// @brief Add a variable to the internal storage
  ///
  /// Variables are saved as properties in this component
  /// This function can be called multiple times per entry.
  void set(const std::string& var_name, const Real& var_values);

  /// @brief Add a variable to the internal storage
  ///
  /// Variables are saved as properties in this component
  /// This function can be called multiple times per entry.
  void set(const std::string& var_name, const std::vector<Real>& var_values);

  /// @brief Configure the point interpolator
  void configure_point_interpolator();

private: // data

  Handle<mesh::Dictionary>            m_dict;                ///< Dictionary to interpolate
  Handle<mesh::PointInterpolator>     m_point_interpolator;  ///< Interpolator for one point
  Handle< math::VariablesDescriptor > m_variables;           ///< Variable description

};

////////////////////////////////////////////////////////////////////////////////

/// @brief ProbePostProcessor class to attach to a probe
///
/// This allows to add/manipulate data from a probe, or perform another
/// action such as logging a variable to a History component, ...
/// @author Willem Deconinck
class solver_actions_API ProbePostProcessor : public common::Action {
public:

  /// Contructor
  /// @param name of the component
  ProbePostProcessor ( const std::string& name );

  /// Virtual destructor
  virtual ~ProbePostProcessor() {}

  /// Get the class name
  static std::string type_name () { return "ProbePostProcessor"; }

  virtual void execute() = 0;

  /// @brief Set in the current entry the variable with given name to a given value
  ///
  /// Variables are saved as properties in this component
  /// This function can be called multiple times per entry.
  void set(const std::string& var_name, const Real& var_values);

  /// @brief Set in the current entry the vector of variables with given name to a given value
  ///
  /// Variables are saved as properties in this component
  /// This function can be called multiple times per entry.
  void set(const std::string& var_name, const std::vector<Real>& var_values);

protected:

  Handle<Probe> m_probe;
};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_Probe_hpp
