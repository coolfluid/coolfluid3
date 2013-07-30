// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_BC_hpp
#define cf3_solver_BC_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"
#include "common/OptionList.hpp"
#include "common/Option.hpp"
#include "physics/MatrixTypes.hpp"
#include "solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

// Forward declarations
namespace cf3 {
  namespace mesh { 
    class Dictionary; 
    class Field; 
  }
  namespace solver {
    class Time;
  }
}

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

class solver_API BC : public common::Action
{
public:
  BC( const std::string& name );

  virtual ~BC();

  static std::string type_name() { return "BC"; }

  const Handle<mesh::Dictionary>& fields()  { return m_fields; }
  const Handle<mesh::Field>& solution()   { return m_solution; }
  const Handle<mesh::Dictionary>& bdry_fields()  { return m_bdry_fields; }
  const Handle<mesh::Field>& bdry_solution()   { return m_bdry_solution; }
  const Handle<mesh::Field>& bdry_solution_gradient()   { return m_bdry_solution_gradient; }
  const Handle<solver::Time>& time() const { return m_time; }

//  template <typename T>
//  common::Option& add_linked_option(Component& component, const std::string& name, T& value);

//  virtual void add_configuration_options(Component& component);

  virtual void create_fields();

protected:

  std::vector< Handle<Component> > m_regions;
  Handle<mesh::Dictionary>         m_fields;
  Handle<mesh::Field>              m_solution;
  Handle<mesh::Dictionary>         m_bdry_fields;
  Handle<mesh::Field>              m_bdry_solution;
  Handle<mesh::Field>              m_bdry_solution_gradient;
  Handle<solver::Time>             m_time;

};

////////////////////////////////////////////////////////////////////////////////

//template <typename T>
//inline common::Option& BC::add_linked_option(Component& component, const std::string& name, T& value)
//{
//  if ( !options().check(name) )
//  {
//    options().add(name,value)
//        .link_to(&value);
//  }
//  if ( &component != this )
//  {
//    if ( component.options().check(name) )
//    {
//      component.options()[name]
//          .link_option(options().option_ptr(name));
//      options()[name] = component.options()[name].value();
//    }
//    else
//    {
//      component.options().add(name,value)
//        .link_option(options().option_ptr(name));
//    }
//  }
//  return component.options()[name];
//}

//////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_BC_hpp
