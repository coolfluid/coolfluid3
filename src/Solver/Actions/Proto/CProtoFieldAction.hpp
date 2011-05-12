// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_CProtoFieldAction_hpp
#define CF_Solver_Actions_Proto_CProtoFieldAction_hpp

#include <set>

#include <boost/scoped_ptr.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/mpl/for_each.hpp>

#include "Common/CAction.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CNodes.hpp"

#include "Solver/CEigenLSS.hpp"

#include "ElementMatrix.hpp"
#include "PhysicalModel.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {
  
/// Abstract base class that encapsulates proto expressions that will operate on fields of a given region
template<typename ExprT>
class CProtoFieldAction : public Common::CAction
{
public:
  typedef boost::shared_ptr< CProtoFieldAction<ExprT> > Ptr;
  typedef boost::shared_ptr< CProtoFieldAction<ExprT> const> ConstPtr;

  CProtoFieldAction(const std::string& name) : Common::CAction(name)
  {
  }

  static std::string type_name() { return "CProtoFieldAction"; }

  /// Set the expression
  /// @param expr The proto expression to set
  /// @param option_owner Component that will store any options that are generated. Generated options are added and connected to their respective triggers.
  /// If an option with the same name existed, it is not replaced and the trigger is added.
  /// @param physical_model Keeps track of the different variables that are shared among expressions, and knows which variables are used in an equation
  /// The model is updated based on the form of the supplied expression
  void set_expression(const ExprT& expr, Common::Component& option_owner, PhysicalModel& physical_model, Common::OptionURI::Ptr region_option)
  {
    // deep-copy the expression
    m_expr.reset(new CopiedExprT(boost::proto::deep_copy(expr)));
    
    // Store the variables
    CopyNumberedVars<VariablesT> ctx(m_variables);
    boost::proto::eval(expr, ctx);
    
    // Register the variables and create the options to allow user configuration
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >( RegisterVariables(physical_model, option_owner, m_variables) );
    
    // Create an option to set the region, unless a non-null one was supplied
    if(region_option)
    {
      m_region_path = region_option;
    }
    else
    {
      m_region_path = boost::dynamic_pointer_cast<Common::OptionURI>( properties().template add_option<Common::OptionURI>("Region", "Region to loop over", std::string()) );
      m_region_path.lock()->supported_protocol(CF::Common::URI::Scheme::CPATH);
      m_region_path.lock()->mark_basic();
    }
    
    raise_event("tree_updated");
  }

  /// Returns a reference to the region that we want to run an expression for
  Mesh::CRegion& root_region()
  {
    return *access_component_ptr(m_region_path.lock()->value_str())->as_ptr<Mesh::CRegion>();
  }
  
  /// Returns a reference to the region that we want to run an expression for
  const Mesh::CRegion& root_region() const
  {
    return *access_component_ptr(m_region_path.lock()->value_str())->as_ptr<Mesh::CRegion>();
  }

protected:
  // Number of variables
  typedef typename ExpressionProperties<ExprT>::NbVarsT NbVarsT;
  
  // Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;
  
  // True for the variables that are stored
  typedef typename EquationVariables<ExprT, NbVarsT>::type EquationVariablesT;

  // type of the copied expression
  typedef typename boost::proto::result_of::deep_copy<ExprT>::type CopiedExprT;

  /// Copy of each variable in the expression
  VariablesT m_variables;
  
  /// Copy of the expression
  boost::scoped_ptr<CopiedExprT> m_expr;

private:
  /// Link to the option with the path to the region to loop over
  boost::weak_ptr<Common::OptionURI> m_region_path;
  
  /// Register variables in a physical model and store the options in the option owner
  struct RegisterVariables
  {
    RegisterVariables(PhysicalModel& physical_model, Component& option_owner, VariablesT& variables) :
      m_physical_model(physical_model),
      m_option_owner(option_owner),
      m_variables(variables)
    {
    }
    
    /// Operator taking an MPL integral constant as type
    template<typename I>
    void operator()(const I&) const
    {
      apply(boost::fusion::at<I>(m_variables), boost::mpl::at<EquationVariablesT, I>::type::value);
    }
    
    /// non-op for non-existing variables in the list
    void apply(const boost::mpl::void_&, const bool) const
    {
    }
    
    /// Register a scalar
    void apply(ScalarField& field, const bool is_equation_var) const
    {
      field.add_options(m_option_owner);
      m_physical_model.register_variable(field.internal_name(), PhysicalModel::SCALAR, is_equation_var);
    }
    
    /// Register a vector field
    void apply(VectorField& field, const bool is_equation_var) const
    {
      field.add_options(m_option_owner);
      m_physical_model.register_variable(field.internal_name(), PhysicalModel::VECTOR, is_equation_var);
    }
    
  private:
    PhysicalModel& m_physical_model;
    Component& m_option_owner;
    VariablesT& m_variables;
  };
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_CProtoFieldAction_hpp
