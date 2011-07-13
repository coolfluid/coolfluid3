// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Solver_Actions_Proto_LSSProxy_hpp
#define CF_Solver_Actions_Proto_LSSProxy_hpp

#include <boost/proto/core.hpp>

#include "Common/OptionComponent.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Physics/PhysModel.hpp"

/// @file
/// Proxy for a linear system, providing access to the linear system solver and physical model components when needed

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

class LSSProxy : boost::noncopyable
{
public:
  /// Construction using references to the actual components (mainly useful in utests or other non-dynamic code)
  /// Using this constructor does not use dynamic configuration through options
  LSSProxy(CEigenLSS& lss, Physics::PhysModel& physical_model);
  
  /// Construction using the options that define the LSS and physical model
  /// Triggers are attached to the options to ensure the LSS and physical model always correspond
  /// to the current settings
  LSSProxy(Common::Option& lss_option, Common::Option& physical_model_option);
  
  /// Return the current Linear System Solver
  CEigenLSS& lss();
  
  /// Return the current physical model
  Physics::PhysModel& physical_model();
  
private:
  /// Link to the LSS option, if any
  boost::weak_ptr< Common::OptionComponent<CEigenLSS> > m_lss_option;
  /// Link to the physical model option, if any
  boost::weak_ptr< Common::OptionComponent<Physics::PhysModel> > m_physical_model_option;

  /// LSS (raw pointer for performance)
  /// There is no way these go out-of-scope during expression execution
  CEigenLSS* m_lss;
  /// Physical model (raw pointer for performance)
  Physics::PhysModel* m_physical_model;
  
  /// Trigger for LSS change
  void trigger_lss();
  
  /// Trigger for physical model change
  void trigger_physical_model();
};

/// Wrap an LSSProxy, and add a Tag template parameter to distinguish between the different matrices of a linear system
template<typename TagT>
struct LSSComponent
{
  typedef TagT tag_type;
  
  LSSComponent(LSSProxy& a_lss_proxy) :
    m_lss_proxy(&a_lss_proxy)
  {
  }
  
  /// Proxy providing access to the LSS and physical model components
  LSSProxy& lss_proxy() const
  {
    return *m_lss_proxy;
  }
  
private:
  mutable LSSProxy* m_lss_proxy;
};

/// Proto terminal that can hold an LSSProxy
template<typename TagT>
struct LSSComponentTerm : boost::proto::extends< typename boost::proto::terminal< LSSComponent<TagT> >::type, LSSComponentTerm<TagT> >
{
  typedef boost::proto::extends< typename boost::proto::terminal< LSSComponent<TagT> >::type, LSSComponentTerm<TagT> > base_type;
  
  LSSComponentTerm(LSSProxy& a_lss_proxy) :
    base_type(boost::proto::make_expr<boost::proto::tag::terminal>(LSSComponent<TagT>(a_lss_proxy)))
  {
  }
};

/// Stores an LSS proxy at the back of the state
struct PushLSSProxy :
  boost::proto::transform< PushLSSProxy >
{
  template<typename ExprT, typename State, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, State, DataT>
  {
    typedef typename impl::state& result_type;

    result_type operator ()(
                typename impl::expr_param expr // The assignment expression
              , typename impl::state_param state // should be the element matrix, i.e. RHS already evaluated
              , typename impl::data_param // data associated with element loop
    ) const
    {
      state.push_back(&boost::proto::value(expr).lss_proxy());
      return state;
    }
  };
};

/// Collects the LSSProxies appearing in the expression, storing them in a vector that is passed as state parameter
struct CollectLSSProxies :
  boost::proto::or_
  <
    boost::proto::when // scalar field used as equation in an element matrix
    <
      boost::proto::terminal< LSSComponent<boost::proto::_> >,
      PushLSSProxy
    >,
    boost::proto::when
    <
      boost::proto::terminal<boost::proto::_>,
      boost::proto::_state
    >,
    boost::proto::when
    <
      boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >
    , boost::proto::fold
      <
        boost::proto::_, boost::proto::_state, CollectLSSProxies
      >
    >
  >
{
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_LSSProxy_hpp
