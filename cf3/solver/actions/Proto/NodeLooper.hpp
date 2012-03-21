// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_NodeLooper_hpp
#define cf3_solver_actions_Proto_NodeLooper_hpp

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>

#include "NodeData.hpp"
#include "NodeGrammar.hpp"

/// @file
/// Loop over the nodes for a region

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Matches expressions that need to be wrapped in an extension before they can be evaluated (i.e. Eigen products)
struct WrappableNodeExpressions :
    boost::proto::multiplies<boost::proto::_, boost::proto::_>
{
};

/// Loop over nodes, when the dimension is known
template<typename ExprT, typename NbDimsT>
struct NodeLooperDim
{
  /// Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;
  
  typedef NodeData<VariablesT, NbDimsT> DataT;
  
  NodeLooperDim(const ExprT& expr, mesh::Region& region, VariablesT& variables) :
    m_expr(expr),
    m_region(region),
    m_variables(variables)
  {
  }
  
  
  /// Domain ued for extended expressions
  struct NodesDomain;
  
  /// Wraps a given expression, so the value that it represents can be stored inside the expression itself
  template<typename ProtoExprT>
  struct NodesExpressionStored :
    boost::proto::extends<ProtoExprT, NodesExpressionStored<ProtoExprT>, NodesDomain>
  {
    typedef boost::proto::extends<ProtoExprT, NodesExpressionStored<ProtoExprT>, NodesDomain> base_type;
    
    typedef typename boost::remove_const<typename boost::remove_reference
    <
      typename boost::result_of<NodeGrammar(const ProtoExprT&, int, DataT&)>::type
    >::type>::type ValueT;
    
    explicit NodesExpressionStored(ProtoExprT const &expr = ProtoExprT())
      : base_type(expr)
    {
    }
    
    /// Temporary storage for the result of the expression
    mutable ValueT value;
  };
  
  template<typename T>
  struct NodesExpression;
  
  template<bool B, typename ProtoExprT>
  struct SelectWrapper;
  
  template<typename ProtoExprT>
  struct SelectWrapper<false, ProtoExprT>
  {
    typedef boost::proto::extends<ProtoExprT, NodesExpression<ProtoExprT>, NodesDomain> type;
  };
  
  template<typename ProtoExprT>
  struct SelectWrapper<true, ProtoExprT>
  {
    typedef NodesExpressionStored<ProtoExprT> type;
  };
  
  template<typename ProtoExprT>
  struct NodesExpression :
    SelectWrapper<boost::proto::matches<ProtoExprT, WrappableNodeExpressions>::value, ProtoExprT>::type
  {
    typedef typename SelectWrapper<boost::proto::matches<ProtoExprT, WrappableNodeExpressions>::value, ProtoExprT>::type base_type;
    
    explicit NodesExpression(ProtoExprT const &expr = ProtoExprT())
      : base_type(expr)
    {
    }
  };
  
  struct NodesDomain :
    boost::proto::domain< boost::proto::generator<NodesExpression> >
  {
  };
  
  struct WrapExpression :
    boost::proto::or_
    <
      boost::proto::when
      <
        boost::proto::multiplies<boost::proto::_, boost::proto::_>,
        boost::proto::functional::make_expr<boost::proto::tag::multiplies, NodesDomain>
        (
          WrapExpression(boost::proto::_left), WrapExpression(boost::proto::_right)
        )
      >,
      boost::proto::nary_expr< boost::proto::_, boost::proto::vararg<WrapExpression> >
    >
  {};
  
  void operator()() const
  {
    // Create data used for the evaluation
#warning Call to find_parent_component in node-loop exposed (used to be in now removed "Region::geometry_fields()" )
    const mesh::Field& coordinates = common::find_parent_component<mesh::Mesh>(m_region).geometry_fields().coordinates();
    DataT node_data(m_variables, m_region, coordinates, m_expr);
    
    // Wrap things up so that we can store the intermediate product results
    do_run(WrapExpression()(m_expr, 0, node_data), node_data);
  }
  
private:
  template<typename FilteredExprT>
  void do_run(const FilteredExprT& expr, DataT& data) const
  {
    NodeGrammar grammar;
    
    std::vector<Uint> nodes;
    const mesh::Field& coordinates = common::find_parent_component<mesh::Mesh>(m_region).geometry_fields().coordinates();
    make_node_list(m_region, coordinates, nodes);
    
    const Uint nb_nodes = nodes.size();
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      data.set_node(nodes[i]);
      grammar(expr, 0, data); // The "0" is the proto state, which is unused at the top-level expression
    }
  }
  
  const ExprT& m_expr;
  mesh::Region& m_region;
  VariablesT& m_variables;
};

/// Loop over nodes, using static-sized vectors to store coordinates
template<typename ExprT>
struct NodeLooper
{
  /// Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;
  
  NodeLooper(const ExprT& expr, mesh::Region& region, VariablesT& variables) :
    m_expr(expr),
    m_region(region),
    m_variables(variables)
  {
  }
  
  template<typename NbDimsT>
  void operator()(const NbDimsT&)
  {
    common::Table<Real>& coords = common::find_parent_component<mesh::Mesh>(m_region).geometry_fields().coordinates();
    if(NbDimsT::value != coords.row_size())
      return;
  
    // Execute with known dimension
    NodeLooperDim<ExprT, NbDimsT>(m_expr, m_region, m_variables)();
  }
  
private:
  
  const ExprT& m_expr;
  mesh::Region& m_region;
  VariablesT& m_variables;
};
  
/// Visit all nodes used by root_region exactly once, executing expr
/// @param variable_names Name of each of the variables, in case a linear system is solved
/// @param variable_sizes Size (number of scalars) that makes up each variable in the linear system, if any
template<typename ExprT>
void for_each_node(mesh::Region& root_region, const ExprT& expr)
{
  // IF COMPILATION FAILS HERE: the espression passed is invalid
  BOOST_MPL_ASSERT_MSG(
    (boost::proto::matches<ExprT, NodeGrammar>::value),
    INVALID_NODE_EXPRESSION,
    (NodeGrammar));
  
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;
  
  VariablesT vars;
  CopyNumberedVars<VariablesT> ctx(vars);
  boost::proto::eval(expr, ctx);
  
  boost::mpl::for_each< boost::mpl::range_c<Uint, 1, 4> >( NodeLooper<ExprT>(expr, root_region, vars) );
}

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_NodeLooper_hpp
