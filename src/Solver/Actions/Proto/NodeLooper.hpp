// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_NodeLooper_hpp
#define CF_Solver_Actions_Proto_NodeLooper_hpp

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/sequence.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector_c.hpp>

#include "NodeData.hpp"
#include "NodeGrammar.hpp"

/// @file
/// Loop over the nodes for a region

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Loop over nodes, using static-sized vectors to store coordinates
template<typename ExprT>
struct NodeLooper
{
  /// Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;
  
  NodeLooper(const ExprT& expr, Mesh::CRegion& region, VariablesT& variables) :
    m_expr(expr),
    m_region(region),
    m_variables(variables)
  {
  }

  
  template<typename NbDimsT>
  void operator()(const NbDimsT&)
  {
    Mesh::CTable<Real>& coords = m_region.nodes().coordinates();
    if(NbDimsT::value != coords.row_size())
      return;
    
    // Create data used for the evaluation
    NodeData<VariablesT, NbDimsT> node_data(m_variables, m_region, coords);
    
    // Grammar used for the evaluation
    NodeGrammar grammar;
    
    std::vector<Uint> nodes;
    make_node_list(m_region, coords, nodes);
    const Uint nb_nodes = nodes.size();
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      node_data.set_node(nodes[i]);
      grammar(m_expr, 0, node_data); // The "0" is the proto state, which is unused at the top-level expression
    }
  }
  
private:
  const ExprT& m_expr;
  Mesh::CRegion& m_region;
  VariablesT& m_variables;
};
  
/// Visit all nodes used by root_region exactly once, executing expr
template<typename ExprT>
void for_each_node(Mesh::CRegion& root_region, const ExprT& expr)
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
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_NodeLooper_hpp
