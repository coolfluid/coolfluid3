// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ProtoAction_hpp
#define cf3_solver_actions_Proto_ProtoAction_hpp

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "solver/Action.hpp"

namespace cf3 {
  namespace common { template<typename T> class OptionComponent; }
  namespace mesh { class Region; }
  namespace physics { class PhysModel; }
namespace solver {
namespace actions {
namespace Proto {

class Expression;

/// Class to encapsulate Proto actions
class ProtoAction : public solver::Action
{
public:



  ProtoAction(const std::string& name);

  ~ProtoAction();

  static std::string type_name() { return "ProtoAction"; }

  void execute();

  /// Set the expression. The action retains ownership of the supplied expression
  /// @param expression The proto expression to set
  void set_expression(const boost::shared_ptr<Expression> & expression);

  /// Append the tags used in the expression
  void insert_field_info(std::map<std::string, std::string>& tags) const;

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

/// Create a new ProtoAction, immediatly setting the expression
boost::shared_ptr< ProtoAction > create_proto_action(const std::string& name, const boost::shared_ptr< Expression >& expression);

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ProtoAction_hpp
