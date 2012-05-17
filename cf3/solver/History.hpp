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
/// An API is offered to easily store the history of Real variables, or vectors
/// of Real variables, and log to file.
///
/// History is stored internally using a common::Table<Real> .
/// An optional (default=ON) logging facility is provided to log the history to
/// file at every new entry.
/// The file format is Tab Separated Values (extension tsv)
///
/// Any number of variables can be added after logging started. This will cause
/// The history file to be rewritten, including the new variables, putting zero's
/// for the non-existent past entries.
///
/// Example:\n
/// @code
/// boost::shared_ptr<History> history = allocate_component<History>("history");
///
/// history->set("iter",1);   // Create new variable "iter", and store it as a property
/// history->save_entry();    // Because new variable: Resize table , create buffer. Then store property "iter" in buffer, write to file
///
/// history->set("iter",2);   // No new variable created, but property "iter" changed
/// history->save_entry();    // Store property "iter" in buffer, write to file
///
/// history->set("iter",3);   // No new variable created, but property "iter" changed
/// history->set("time",0.1); // Create new variable "time", and store it as a property
/// history->save_entry();    // Because new variable: Resize table , create buffer. Then store properties "iter" and "time" in buffer, write to file
/// @endcode
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

  /// @name High level API
  //@{

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

  /// @brief Finalize an entry in history, save it, and write it optionally (default=ON) to file
  ///
  /// - The entry is assembled from the properties that are set using the function set().
  /// - In case new variables were created, resize the table, and create new buffer.
  /// - The entry is then saved in the buffer
  /// - The entry is optionally (default=ON) saved to file. In case of new variables, the file
  ///   gets recreated.
  void save_entry();

  //@}

  /// @name SIGNALS
  //@{
  /// @brief Write the history to file, signal
  void signal_write(common::SignalArgs& args);

  /// @brief Write the history to file, signature
  void signature_write(common::SignalArgs& args);
  //@}

  /// @brief Write the history to file
  void write_file(boost::filesystem::fstream& file);

  /// @brief Read access to the table storing the history
  /// @note This flushes the buffer first, so that most recent information is available
  Handle<common::Table<Real> const> table();

  /// @brief Information of every variable stored in history
  Handle<math::VariablesDescriptor const> variables() const;


  /// @brief Flush the buffer in the table
  void flush();

  /// @brief make a Entry object that can be written to any output stream
  HistoryEntry entry() const;

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

/// @brief Helper class to output one entry to an output stream
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
