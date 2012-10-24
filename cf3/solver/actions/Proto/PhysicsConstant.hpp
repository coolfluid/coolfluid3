// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_PhysicsConstant_hpp
#define cf3_solver_actions_Proto_PhysicsConstant_hpp

#include <string>

#include <boost/proto/core.hpp>

#include "Terminals.hpp"

/// @file
/// Provide access to constants from a physical model

class C;
namespace cf3 {

  namespace common { class PropertyList; }

namespace solver {
namespace actions {
namespace Proto {



/// Refers to a value from the physical model
struct PhysicsConstant
{
  /// @param p_name: Name of the constant to refer to
  PhysicsConstant(const std::string& p_name):
    name(p_name)
  {
  }

  std::string name;
};

/// Storage for the values pointed to by a PhysicsConstant
struct PhysicsConstantStorage
{
  typedef std::map<std::string, Real> ScalarsT;

  ScalarsT& scalars()
  {
    return m_scalars;
  }

  ScalarsT m_scalars;
};

/// Transform to replace an occurance of PhysicsConstant with a reference to its value
struct ReplacePhysicsConstant :
  boost::proto::transform< ReplacePhysicsConstant >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::make_expr
    <
      boost::proto::tag::terminal,
      Real&
    >::type result_type;

    typedef PhysicsConstantStorage::ScalarsT ValuesT;

    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param values // state parameter
              , typename impl::data_param
    ) const
    {
      const PhysicsConstant& constant = boost::proto::value(expr);
      std::pair<typename ValuesT::iterator, bool> insert_result = values.scalars().insert(std::make_pair(constant.name, 0.));

      return boost::proto::make_expr<boost::proto::tag::terminal>(boost::ref(insert_result.first->second));
    }
  };
};

/// Grammar replacing PhysicsConstants in an expression.
struct ReplacePhysicsConstants :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::terminal< PhysicsConstant >,
      ReplacePhysicsConstant
    >,
    boost::proto::terminal<boost::proto::_>,
    boost::proto::nary_expr< boost::proto::_, boost::proto::vararg<ReplacePhysicsConstants> >
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_PhysicsConstant_hpp
