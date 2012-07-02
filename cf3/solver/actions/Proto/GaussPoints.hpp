// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_GaussPoints_hpp
#define cf3_solver_actions_Proto_GaussPoints_hpp

#include <boost/proto/core.hpp>

#include "mesh/Integrators/Gauss.hpp"

/// @file
/// Access to gauss points and weights

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Transform to evaluate Gauss point access
struct GaussPointEval:
boost::proto::transform<GaussPointEval>
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    static const Uint order = boost::remove_reference<ExprT>::type::value;
    typedef typename boost::remove_reference<DataT>::type::SupportT::EtypeT ShapeFunctionT;
    typedef mesh::Integrators::GaussMappedCoords<order, ShapeFunctionT::shape> GaussT;

    typedef const typename GaussT::CoordsT& result_type;

    result_type operator ()(
      typename impl::expr_param expr
      , typename impl::state_param state
      , typename impl::data_param data
    ) const
    {
      return GaussT::instance().coords;
    }
  };
};

/// Transform to evaluate Gauss weight access
struct GaussWeightEval:
boost::proto::transform<GaussWeightEval>
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    static const Uint order = boost::remove_reference<ExprT>::type::value;
    typedef typename boost::remove_reference<DataT>::type::SupportT::EtypeT ShapeFunctionT;
    typedef mesh::Integrators::GaussMappedCoords<order, ShapeFunctionT::shape> GaussT;

    typedef const typename GaussT::WeightsT& result_type;

    result_type operator ()(
      typename impl::expr_param expr
      , typename impl::state_param state
      , typename impl::data_param data
    ) const
    {
      return GaussT::instance().weights;
    }
  };
};

/// Static constant for the Gauss integration order
template<Uint Order>
struct GaussOrder
{
  static const Uint value = Order;
};

template<typename OrderT>
struct GaussPointTag
{
  static const Uint value = OrderT::value;
};

static boost::proto::terminal< GaussPointTag< GaussOrder<1> > >::type gauss_points_1 = {};
static boost::proto::terminal< GaussPointTag< GaussOrder<2> > >::type gauss_points_2 = {};

template<typename OrderT>
struct GaussWeightTag
{
  static const Uint value = OrderT::value;
};

static boost::proto::terminal< GaussWeightTag< GaussOrder<1> > >::type gauss_weights_1 = {};
static boost::proto::terminal< GaussWeightTag< GaussOrder<2> > >::type gauss_weights_2 = {};

struct GaussGrammar :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::terminal< GaussPointTag<boost::proto::_> >,
      GaussPointEval(boost::proto::_value)
    >,
    boost::proto::when
    <
      boost::proto::terminal< GaussWeightTag<boost::proto::_> >,
      GaussWeightEval(boost::proto::_value)
    >
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_GaussPoints_hpp
