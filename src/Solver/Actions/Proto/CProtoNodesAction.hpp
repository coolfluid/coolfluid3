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
#include "PhysicalModel.hpp"

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
    BaseT(name),
    m_physical_model(0)
  {
  }

  static std::string type_name() { return "CProtoNodesAction"; }

  void set_physical_model(const PhysicalModel& model)
  {
    m_physical_model = &model;
  }

  /// Run the expression, looping over all nodes
  virtual void execute()
  {
    // IF COMPILATION FAILS HERE: the espression passed is invalid
    BOOST_MPL_ASSERT_MSG(
      (boost::proto::matches<ExprT, NodeGrammar>::value),
      INVALID_NODE_EXPRESSION,
      (NodeGrammar));
    
    boost::mpl::for_each< boost::mpl::range_c<Uint, 1, 4> >( NodeLooper<typename BaseT::CopiedExprT>(*BaseT::m_expr, BaseT::root_region(), BaseT::m_variables, m_physical_model ? *m_physical_model : PhysicalModel()) );
  }
  
private:
  PhysicalModel const* m_physical_model;
};

/// Returns a configurable CAction object that will execute the supplied expression for all elements
template<typename ExprT>
Common::CAction& build_nodes_action(const std::string& name, Common::Component& parent, Common::Component& option_owner, PhysicalModel& physical_model, const ExprT& expr, Common::OptionURI::Ptr region_option = Common::OptionURI::Ptr())
{
  boost::shared_ptr< CProtoNodesAction<ExprT> > result = parent.create_component_ptr< CProtoNodesAction<ExprT> >(name);
  regist_typeinfo(result.get());
  result->set_expression(expr, option_owner, physical_model, region_option);
  return *result;
}


} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_CProtoNodesAction_hpp
