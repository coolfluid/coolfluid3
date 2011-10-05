// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_CProtoAction_hpp
#define CF_Solver_Actions_Proto_CProtoAction_hpp

#include <set>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "Solver/Action.hpp"

namespace CF {
  namespace Common { template<typename T> class OptionComponent; }
  namespace Mesh { class CRegion; }
  namespace Physics { class PhysModel; }
namespace Solver {
namespace Actions {
namespace Proto {

class Expression;

/// Class to encapsulate Proto actions
class CProtoAction : public Solver::Action
{
public:
  typedef boost::shared_ptr< CProtoAction > Ptr;
  typedef boost::shared_ptr< CProtoAction const> ConstPtr;

  CProtoAction(const std::string& name);

  ~CProtoAction();

  static std::string type_name() { return "CProtoAction"; }

  void execute();

  /// Set the expression. The action retains ownership of the supplied expression
  /// @param expression The proto expression to set
  void set_expression(const boost::shared_ptr<Expression> & expression);

  /// Append the tags used in the expression
  void insert_tags(std::set<std::string>& tags) const;

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

/// Create a new CProtoAction, immediatly setting the expression
CProtoAction::Ptr create_proto_action(const std::string& name, const boost::shared_ptr< Expression >& expression);

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_CProtoAction_hpp
