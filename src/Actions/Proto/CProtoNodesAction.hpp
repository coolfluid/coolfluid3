// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_CProtoNodesAction_hpp
#define CF_Actions_CProtoNodesAction_hpp

#include "Actions/CAction.hpp"

#include "NodeLooper.hpp"

namespace CF {
namespace Actions {
namespace Proto {

/// A CAction that runs a proto expression over all nodes in the configured region
template<typename ExprT>
class CProtoNodesAction : public CAction
{
public:
  typedef boost::shared_ptr< CProtoNodesAction<ExprT> > Ptr;
  typedef boost::shared_ptr< CProtoNodesAction<ExprT> const> ConstPtr;
  
  CProtoNodesAction(const std::string& name) :
    CAction(name)
  {
    m_region_path = boost::dynamic_pointer_cast<Common::OptionURI>( properties().template add_option<Common::OptionURI>("Region", "Region to loop over", std::string()) );
    m_region_path.lock()->supported_protocol("cpath");
    m_region_path.lock()->mark_basic();
  }
  
  static std::string type_name() { return "CProtoNodesAction"; }
  
  /// Set the expression
  void set_expression(const ExprT& expr)
  {
    m_expr = boost::proto::deep_copy(expr);
    // Store the variables
    CopyNumberedVars<VariablesT> ctx(m_variables);
    boost::proto::eval(expr, ctx);
    boost::fusion::for_each(m_variables, AddVariableOptions(get()));
  }
  
  /// Run the expression, looping over all nodes
  virtual void execute()
  {
    Mesh::CRegion& root_region = *look_component<Mesh::CRegion>( m_region_path.lock()->template value<std::string>() );
    
    // Create data used for the evaluation
    NodeData<VariablesT> node_data(m_variables, root_region);
    
    // Grammar used for the evaluation
    NodeGrammar grammar;
    
    Mesh::CTable<Real>* coordinates = Common::find_component_ptr_with_tag<Mesh::CTable<Real> >(root_region,"coordinates").get();
    if(coordinates) // region owns coordinates, so we assume a loop over all nodes
    {
      // Evaluate the expression for each node
      const Uint nb_nodes = coordinates->size();
      for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
      {
        node_data.set_node(node_idx);
        grammar(m_expr, 0, node_data); // The "0" is the proto state, which is unused at the top-level expression
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
        grammar(m_expr, 0, node_data); // The "0" is the proto state, which is unused at the top-level expression
      }
    }
  }
  
private:
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
  
  // type of the copied expression
  typedef typename boost::proto::result_of::deep_copy<ExprT>::type CopiedExprT;
  
  /// Copy of each variable in the expression
  VariablesT m_variables;
  
  /// Copy of the expression
  CopiedExprT m_expr;
  
  /// Link to the option with the path to the region to loop over
  boost::weak_ptr<Common::OptionURI> m_region_path;
};

/// Returns a configurable CAction object that will execute the supplied expression for all elements
template<typename ExprT>
CAction::Ptr build_nodes_action(const std::string& name, Common::Component& parent, const ExprT& expr)
{
  boost::shared_ptr< CProtoNodesAction<ExprT> > result = parent.create_component< CProtoNodesAction<ExprT> >(name);
  result->set_expression(expr);
  return boost::static_pointer_cast<CAction>(result);
}

  
} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_CProtoNodesAction_hpp
