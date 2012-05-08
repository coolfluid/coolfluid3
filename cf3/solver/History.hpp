// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_History_hpp
#define cf3_solver_History_hpp

#include "common/BoostFilesystem.hpp"

#include "common/Table.hpp"

#include "math/VariablesDescriptor.hpp"

#include "solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

class History;
class HistoryEntry;

////////////////////////////////////////////////////////////////////////////////

/// @brief Stores History of variables
///
/// An API is offered to dynamically grow a history using buffers.
/// Any number of variables can be added during runtime.
/// A logging facility is provided to log a history to file in
/// Tab Separated Values format (tsv).
/// Accessing the history triggers a flush of the buffers
/// @author Willem Deconinck
class solver_API History : public common::Component
{
public:

  /// @brief Contructor
  /// @param name of the component
  History ( const std::string& name );

  /// @brief Virtual destructor
  virtual ~History();

  /// @brief Get the class name
  static std::string type_name () { return "History"; }

  /// @brief Set in the current entry the variable with given name to a given value
  void set(const std::string& var_name, const Real& var_values);

  /// @brief Set in the current entry the variable with given name to a given value
  void set(const std::string& var_name, const std::vector<Real>& var_values);

  /// @brief Save the current properties in the buffer
  void save_entry();

  /// @brief Read access to the table storing the history
  /// @note This flushes the buffer first, so that newest information is available
  Handle<common::Table<Real> const> table();

  /// @brief Information of every variable stored in history
  Handle<math::VariablesDescriptor const> variables() const;

  /// @brief Flush the buffer in the table
  void flush();

  /// @brief make a Entry object that can be written to any output stream
  HistoryEntry entry() const;

  /// @brief Write the history to file
  void write_file(boost::filesystem::fstream& file);

  /// @brief Write the history to file, signal
  void signal_write(common::SignalArgs& args);

  /// @brief Write the history to file, signature
  void signature_write(common::SignalArgs& args);

private: // functions

  /// @brief open a file with given URI
  static void open_file(boost::filesystem::fstream& file, const common::URI& file_uri);

  /// @brief resize table and rebuild buffer if needed
  bool resize_if_necessary();

  /// @brief return the log-file header in string format
  std::string file_header() const;

private: // data

  /// Flag to check if the history has to be logged
  bool m_logging;

  /// Log file handle
  boost::filesystem::fstream m_file;

  /// Handle to the table
  Handle< common::Table<Real> > m_table;

  /// The buffer to manipulate m_table
  boost::shared_ptr< common::Table<Real>::Buffer > m_buffer;

  /// Description of the variables
  Handle< math::VariablesDescriptor > m_variables;

  /// Flag to check if variables have been added.
  /// If so, the table needs to be resized.
  bool m_table_needs_resize;

}; // History

////////////////////////////////////////////////////////////////////////////////

class HistoryEntry
{
public:
  /// @brief Constructor
  HistoryEntry(const History& history);

  /// @brief output tab-separated values
  friend std::ostream& operator<< (std::ostream& out, const HistoryEntry& history_entry);

  /// @brief Info for the entry variables
  Handle<math::VariablesDescriptor const> variables() const;

  /// @brief The entry data
  const std::vector<Real>& data() const;

private: // data

  std::vector<Real> m_entry; ///< entry data
  const History& m_history;  ///< history this entry belongs to
};

std::ostream& operator<< ( std::ostream& os, const HistoryEntry& hist_entry );

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_History_hpp
