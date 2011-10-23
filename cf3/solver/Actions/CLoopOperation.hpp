// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Actions_CLoopOperation_hpp
#define cf3_Actions_CLoopOperation_hpp

#include "common/Action.hpp"

#include "solver/Actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  class Elements;
  class Entities;
  template <typename T> class List;
}
namespace solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class solver_Actions_API CLoopOperation : public common::Action
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CLoopOperation> Ptr;
  typedef boost::shared_ptr<CLoopOperation const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CLoopOperation ( const std::string& name );

  /// Virtual destructor
  virtual ~CLoopOperation() {}

  /// Get the class name
  static std::string type_name () { return "CLoopOperation"; }
  
  void select_loop_idx ( const Uint idx ) { m_idx = idx; }

  /// Called before looping to prepare a helper object that caches entries
  /// needed by this operation to perform the loop efficiently.
  /// Typically accesses components and stores their address, since they are not expected to change over looping.
  virtual void create_loop_helper ( mesh::Elements& geometry_elements ) {}
  
  void set_elements(mesh::Entities& elements);
  
  bool can_start_loop() { return m_can_start_loop; }
  
protected: // functions

  Uint idx() const { return m_idx; }  
  
  mesh::Entities const& elements() const { cf3_assert( is_not_null(m_elements.lock()) ); return *m_elements.lock(); }
  mesh::Entities& elements() { cf3_assert( is_not_null(m_elements.lock()) ); return *m_elements.lock(); }

  bool m_can_start_loop;
  
private: // functions 

  bool m_call_config_elements;
  void config_elements();

private: // data
  
  Uint m_idx;

  boost::weak_ptr<mesh::Entities>  m_elements;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Actions_CLoopOperation_hpp
