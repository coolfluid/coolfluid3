// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_Proto_NodeLooper_hpp
#define CF_Actions_Proto_NodeLooper_hpp

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
namespace Actions {
namespace Proto {

/// Visit all nodes used by root_region exactly once, executing expr
template<typename ExprT>
void for_each_node(Mesh::CRegion& root_region, const ExprT& expr)
{
  // Number of variables (integral constant)
  typedef typename boost::result_of<ExprVarArity(ExprT)>::type NbVarsT;
  
  // init empty vector that will store variable indices
  typedef boost::mpl::vector_c<Uint> EmptyRangeT;
  
  // Fill the vector with indices 0 to 9, so we allow 10 different (field or node related) variables in an expression
  typedef typename boost::mpl::copy<
      boost::mpl::range_c<int,0,NbVarsT::value>
    , boost::mpl::back_inserter< EmptyRangeT >
    >::type NbVarsRangeT;
  
  // Get the type for each variable that is used, or set to boost::mpl::void_ for unused indices
  typedef typename boost::mpl::transform<NbVarsRangeT, DefineTypeOp<boost::mpl::_1, ExprT > >::type VarTypesT;
  
  // Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename boost::fusion::result_of::as_vector<VarTypesT>::type VariablesT;
  
  // Store the variables
  VariablesT vars;
  CopyNumberedVars<VariablesT> ctx(vars);
  boost::proto::eval(expr, ctx);
  
  // Create data used for the evaluation
  NodeData<VariablesT> node_data(vars, root_region);
  
  // Grammar used for the evaluation
  NodeGrammar grammar;
  
  Mesh::CTable<Real>* coordinates = Common::find_component_ptr_with_tag<Mesh::CTable<Real> >(root_region, "coordinates").get();
  if(coordinates) // region owns coordinates, so we assume a loop over all nodes
  {
    // Evaluate the expression for each node
    const Uint nb_nodes = coordinates->size();
    for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
    {
      node_data.set_node(node_idx);
      grammar(expr, 0, node_data); // The "0" is the proto state, which is unused at the top-level expression
    }
  }
  else // no coordinates found, assert that all CElements share the same coords, and use it to construct a list of nodes to visit
  {
    const Mesh::CTable<Real>& coords = extract_coordinates(root_region);
    std::vector<Uint> nodes;
    make_node_list(root_region, coords, nodes);
    const Uint nb_nodes = nodes.size();
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      node_data.set_node(nodes[i]);
      grammar(expr, 0, node_data); // The "0" is the proto state, which is unused at the top-level expression
    }
  }
}

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_Proto_NodeLooper_hpp
