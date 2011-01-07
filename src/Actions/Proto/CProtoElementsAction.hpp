// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_CProtoElementsAction_hpp
#define CF_Actions_CProtoElementsAction_hpp

#include "Actions/CAction.hpp"

#include "Common/CBuilder.hpp"

#include "Mesh/SF/Types.hpp"

#include "ElementLooper.hpp"

namespace CF {
namespace Actions {
namespace Proto {
 
/// A CAction that runs a proto expression over all cells referred to by the support.
/// The support is deduced from the expression itself
template<typename ExprT>
class CProtoElementsAction : public CAction
{
public:
  typedef boost::shared_ptr< CProtoElementsAction<ExprT> > Ptr;
  typedef boost::shared_ptr< CProtoElementsAction<ExprT> const> ConstPtr;
  
  CProtoElementsAction(const std::string& name) :
    CAction(name)
  {
  }
  
  static std::string type_name() { return "CProtoElementsAction"; }
  
  /// Set the expression
  void set_expression(const ExprT& expr)
  {
    m_expr = boost::proto::deep_copy(expr);
    // Store the variables
    CopyNumberedVars<VariablesT> ctx(m_variables);
    boost::proto::eval(expr, ctx);
    boost::fusion::for_each(m_variables, AddVariableOptions(follow()));
    raise_event("tree_updated");
  }
  
  /// Return the root region
  Mesh::CRegion& root_region()
  {
    return (*boost::fusion::find<ConstNodes>(m_variables)).region();
  }
  
  /// Run the expression, looping over all elements
  virtual void execute()
  { 
    // Traverse all CElements under the root and evaluate the expression
    BOOST_FOREACH(Mesh::CElements& elements, Common::find_components_recursively<Mesh::CElements>(root_region()))
    {
      // Create an expression runner, taking rather many template arguments, and use it to run the expression.
      // This recurses through the variables, selecting the appropriate shape function for each variable
      ExpressionRunner
      <
        Mesh::SF::VolumeTypes, // We consider volume functions for element expressions
        CopiedExprT, // type of the expression
        boost::mpl::void_, // No support by default
        VariablesT, // type of the fusion vector with the variables
        boost::fusion::vector0<>, // Start with an empty vector for the per-variable data
        NbVarsT, // number of variables
        boost::mpl::int_<0>, // Start index, as MPL integral constant
        typename IsSFDependent< typename boost::remove_reference< typename boost::fusion::result_of::front<VariablesT>::type >::type >::type // Determine if the first var needs a shape function
      >(m_variables, m_expr, elements).run();
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
};

/// Returns a configurable CAction object that will execute the supplied expression for all elements
template<typename ExprT>
CAction::Ptr build_elements_action(const std::string& name, Common::Component& parent, const ExprT& expr)
{
  boost::shared_ptr< CProtoElementsAction<ExprT> > result = parent.create_component< CProtoElementsAction<ExprT> >(name);
  regist_typeinfo(result.get());
  result->set_expression(expr);
  return boost::static_pointer_cast<CAction>(result);
}

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_CProtoElementsAction_hpp
