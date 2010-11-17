// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_CProtoElementsAction_hpp
#define CF_Actions_CProtoElementsAction_hpp

#include "Actions/CAction.hpp"

#include "ProtoElementLooper.hpp"

namespace CF {
namespace Actions {
namespace Proto {

template<typename ExprT>
class CProtoElementsAction : CAction
{
public:
  CProtoElementsAction(const CName& name) : CAction(name) {}
  
private:
  // Number of variables
  typedef typename boost::result_of<ExprVarArity(ExprT)>::type nb_vars;
  
  // init empty vector that will store variable indices
  typedef boost::mpl::vector_c<Uint> numbers_empty;
  
  // Fill the vector with indices 0 to 9, so we allow 10 different (field or node related) variables in an expression
  typedef typename boost::mpl::copy<
      boost::mpl::range_c<int,0,nb_vars::value>
    , boost::mpl::back_inserter< numbers_empty >
    >::type range_nb_vars;
  
  // Get the type for each variable that is used, or set to boost::mpl::void_ for unused indices
  typedef typename boost::mpl::transform<range_nb_vars, DefineTypeOp<boost::mpl::_1, ExprT > >::type VariableTypeVectorT;
  
  /// type of a fusion vector containing a unique copy of each numbered variable in the expression
  typedef typename boost::fusion::result_of::as_vector<VariableTypeVectorT>::type VariablesT;
  
  /// Copy of each variable in the expression
  VariablesT m_variables;
};

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_CProtoElementsAction_hpp
