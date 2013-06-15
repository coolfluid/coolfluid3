// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_actions_LoopOperation_hpp
#define cf3_actions_LoopOperation_hpp

#include "common/Action.hpp"

#include "solver/actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  template <typename T> class List;
}
namespace mesh {
  class Elements;
  class Entities;
}
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API LoopOperation : public common::Action
{
public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  LoopOperation ( const std::string& name );

  /// Virtual destructor
  virtual ~LoopOperation() {}

  /// Get the class name
  static std::string type_name () { return "LoopOperation"; }

  void select_loop_idx ( const Uint idx ) { m_idx = idx; }

  /// Called before looping to prepare a helper object that caches entries
  /// needed by this operation to perform the loop efficiently.
  /// Typically accesses components and stores their address, since they are not expected to change over looping.
  virtual void create_loop_helper ( mesh::Elements& geometry_elements ) {}

  void set_elements(mesh::Entities& elements);

  bool can_start_loop() { return m_can_start_loop; }

protected: // functions

  Uint idx() const { return m_idx; }

  mesh::Entities const& elements() const { cf3_assert( is_not_null(m_elements) ); return *m_elements; }
  mesh::Entities& elements() { cf3_assert( is_not_null(m_elements) ); return *m_elements; }

  bool m_can_start_loop;

private: // functions

  bool m_call_config_elements;
  void config_elements();

private: // data

  Uint m_idx;

  Handle<mesh::Entities>  m_elements;

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_actions_LoopOperation_hpp
