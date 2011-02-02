// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_CProtoNodesAction_hpp
#define CF_Solver_Actions_Proto_CProtoNodesAction_hpp

#include "Solver/Actions/LibActions.hpp"

#include "CProtoFieldAction.hpp"
#include "NodeLooper.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// A CAction that runs a proto expression over all nodes in the configured region
template<typename ExprT>
class CProtoNodesAction : public CProtoFieldAction<ExprT>
{
  typedef CProtoFieldAction<ExprT> BaseT;
public:
  typedef boost::shared_ptr< CProtoNodesAction<ExprT> > Ptr;
  typedef boost::shared_ptr< CProtoNodesAction<ExprT> const> ConstPtr;

  CProtoNodesAction(const std::string& name) :
    BaseT(name)
  {
  }

  static std::string type_name() { return "CProtoNodesAction"; }

  /// Run the expression, looping over all nodes
  virtual void execute()
  {
    // IF COMPILATION FAILS HERE: the espression passed is invalid
    BOOST_MPL_ASSERT_MSG(
      (boost::proto::matches<ExprT, NodeGrammar>::value),
      INVALID_NODE_EXPRESSION,
      (NodeGrammar));
    
    boost::mpl::for_each< boost::mpl::range_c<Uint, 1, 4> >( NodeLooper<typename BaseT::CopiedExprT>(*BaseT::m_expr, BaseT::root_region(), BaseT::m_variables) );
  }
};

/// Returns a configurable CAction object that will execute the supplied expression for all elements
template<typename ExprT>
Common::CAction::Ptr build_nodes_action(const std::string& name, Common::Component& parent, const ExprT& expr)
{
  boost::shared_ptr< CProtoNodesAction<ExprT> > result = parent.create_component< CProtoNodesAction<ExprT> >(name);
  regist_typeinfo(result.get());
  result->set_expression(expr);
  return boost::static_pointer_cast<Common::CAction>(result);
}


} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_CProtoNodesAction_hpp
