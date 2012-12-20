// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_FieldSync_hpp
#define cf3_solver_actions_Proto_FieldSync_hpp

#include "mesh/Field.hpp"

/// @file
/// Helper struct to synchronize fields at the end of a loop

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Helper struct to synchronize fields at the end of a loop
class FieldSynchronizer : public boost::noncopyable
{
public:
  
  /// Singleton implementation
  static FieldSynchronizer& instance();

  /// Insert a field to synchronize
  void insert(mesh::Field& f);

  /// Sync fields and clear the list
  void synchronize();

private:
  FieldSynchronizer();

  // Fields to synchronize are put in a map using the URI as key, so ensure they are sorted the same way
  // on each cpu.
  typedef std::map< std::string, Handle<mesh::Field> > FieldsT;
  FieldsT m_fields;
};


} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_FieldSync_hpp
