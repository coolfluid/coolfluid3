// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Actions_Proto_BlockAccumulator_hpp
#define CF_Actions_Proto_BlockAccumulator_hpp

#include<iostream>

#include <boost/proto/proto.hpp>

#include "Math/MatrixTypes.hpp"

#include "Solver/CEigenLSS.hpp"

#include "Terminals.hpp"
#include "Transforms.hpp"

/// @file
/// System matrix block accumultation. Current prototype uses dense a dense Eigen matrix and is purely for proof-of-concept

namespace CF {
namespace Actions {
namespace Proto {

/// Encapsulate a system matrix
struct LSS : OptionVariable
{ 
  LSS() : OptionVariable("aLSS", "Linear System Solver")
  {
  }
  
  LSS(const std::string& aname, Solver::CEigenLSS::Ptr l = Solver::CEigenLSS::Ptr()) : OptionVariable(aname, "Linear System Solver"), m_lss(l)
  {
  }
  
  /// Linear system
  Solver::CEigenLSS& lss()
  {
    return *m_lss.lock();
  }

protected:
  virtual void add_options()
  {
    m_lss_path = add_option<Common::URI>(m_name, m_description, boost::bind(&LSS::on_trigger, this));
    m_lss_path.lock()->supported_protocol("cpath");
    m_physical_model = Common::find_component_ptr<Solver::CPhysicalModel>(*m_owner.lock()->get_parent());
    if(!m_physical_model.expired())
      m_physical_model.lock()->properties()["DOFs"].as_option().attach_trigger( boost::bind(&LSS::trigger_dofs, this) );
  }


private:
  void on_trigger()
  {
    m_lss = m_owner.lock()->look_component<Solver::CEigenLSS>(m_lss_path.lock()->value_str());
    reset();
  }
  
  void trigger_dofs()
  {
    reset();
  }
  
  void reset()
  {
    if(m_lss.expired() || m_lss.lock()->size() == m_physical_model.lock()->nb_dof())
      return;
    lss().resize(m_physical_model.lock()->nb_dof());
    lss().matrix().setZero();
    lss().rhs().setZero();
  }
  
  /// Linear system
  boost::weak_ptr<Solver::CEigenLSS> m_lss;
  boost::weak_ptr<Solver::CPhysicalModel> m_physical_model;
  
  /// Option with the path to the LSS
  boost::weak_ptr<Common::OptionURI> m_lss_path;
};

/// Helper struct for assignment to a matrix
template<typename OpTagT>
struct BlockAssignmentOp;

/// Allowed block assignment operations
struct MatrixAssignOpsCases
{
  template<typename TagT> struct case_  : boost::proto::not_<boost::proto::_> {};
};

/// Macro to easily define =, +=, -=, ...
#define MAKE_ASSIGN_OP(__tagname, __op)    \
template<> \
struct BlockAssignmentOp<boost::proto::tag::__tagname> \
{ \
  template<typename MatrixT, int NbRows, int NbCols, typename ConnectivityT> \
  static void assign(MatrixT& matrix, const Eigen::Matrix<Real, NbRows, NbCols>& rhs, const ConnectivityT& connectivity) \
  { \
    for(Uint i = 0; i != NbRows; ++i) \
      for(Uint j = 0; j != NbCols; ++j) \
        matrix(connectivity[i], connectivity[j]) __op rhs(i, j); \
  } \
}; \
\
template<> \
struct MatrixAssignOpsCases::case_<boost::proto::tag::__tagname> : \
  boost::proto::__tagname<boost::proto::terminal< Var< boost::proto::_, LSS > >, boost::proto::_> \
{};

MAKE_ASSIGN_OP(assign, =)
MAKE_ASSIGN_OP(plus_assign, +=)

#undef MAKE_ASSIGN_OP

/// Primitive transform to handle assignment to an LSS matrix
struct BlockAccumulator :
  boost::proto::transform< BlockAccumulator >
{
  template<typename ExprT, typename State, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, State, DataT>
  {
    /// Contrary to general C++ assignment, assigning to a system matrix doesn't return anything
    typedef void result_type;
    
    /// Tag type represents the actual operation type
    typedef typename boost::proto::tag_of<ExprT>::type OpT;
    
    result_type operator ()(
                typename impl::expr_param expr // The complete expression
              , typename impl::state_param state // should be the element matrix, i.e. RHS already evaluated
              , typename impl::data_param data // data associated with element loop
    ) const
    {
      BlockAssignmentOp<OpT>::assign
      (
        VarValue()(boost::proto::left(expr), state, NumberedData()(boost::proto::left(expr), state, data)).lss().matrix(),
        state,
        data.support().element_connectivity()
      );
    }
  };
};

/// Grammar matching block accumulation expressions
struct BlockAccumulation
  : boost::proto::switch_<MatrixAssignOpsCases>
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_Proto_BlockAccumulator_hpp
