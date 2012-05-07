// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_History_hpp
#define cf3_solver_History_hpp

#include "common/Table.hpp"

#include "math/VariablesDescriptor.hpp"

#include "solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// Stores History of variables
///
/// An API is offered to dynamically grow using buffers.
/// Accessing the history triggers a flush of the buffers
/// @author Willem Deconinck
class solver_API History : public common::Component
{
public:

  /// Contructor
  /// @param name of the component
  History ( const std::string& name );

  /// Virtual destructor
  virtual ~History() {}

  /// Get the class name
  static std::string type_name () { return "History"; }

  /// @brief Set in the current entry the variable with given name to a given value
  void set(const std::string& var_name, const Real& var_values);

  /// @brief Set in the current entry the variable with given name to a given value
  void set(const std::string& var_name, const std::vector<Real>& var_values);

  /// @brief Save the current properties in the buffer
  void save_entry();

  /// @brief Read access to the table storing the history
  /// @note This flushes the buffer first, so that newest information is available
  const Handle<common::Table<Real> const> table();

  /// @brief Flush the buffer in the table
  void flush();

  /// @brief Write the history to file
  void signal_write(common::SignalArgs& args);
  void signature_write(common::SignalArgs& args);

private: // data

  /// Handle to the table
  Handle< common::Table<Real> > m_table;

  /// The buffer to manipulate m_table
  boost::shared_ptr< common::Table<Real>::Buffer > m_buffer;

  /// Description of the variables
  Handle< math::VariablesDescriptor > m_variables;

  bool m_table_needs_resize;

}; // History

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_History_hpp
