// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ConfigurableConstant_hpp
#define cf3_solver_actions_Proto_ConfigurableConstant_hpp

#include <string>

#include <boost/proto/core.hpp>

#include "Terminals.hpp"

/// @file
/// Grammar for node-based expressions

class C;
namespace cf3 {

  namespace common { class PropertyList; }

namespace solver {
namespace actions {
namespace Proto {

/// Refers to a configurable constant value.
template<typename ValueT>
struct ConfigurableConstant
{
  /// This is needed in case ValueT is a fixed-size Eigen matrix
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  typedef ValueT value_type;

  ConfigurableConstant(const std::string& p_name, const std::string& p_description = std::string(), const ValueT p_default = ValueT()) :
    name(p_name),
    description(p_description),
    default_value(p_default)
  {
  }

  std::string name;
  std::string description;
  value_type default_value;
};

/// Returns the map type used to store values of the given type
template<typename T>
struct ConstantStorageType
{
  typedef std::map<std::string, T> type;
};

/// Storage for the values pointed to by a ConfigurableConstant
struct ConstantStorage
{
  typedef std::map<std::string, std::string> DescriptionsT;
  typedef ConstantStorageType<Real>::type ScalarsT;
  typedef ConstantStorageType<RealVector>::type VectorsT;
  typedef ConstantStorageType< std::vector<Real> >::type VectorsProxiesT;

  ScalarsT& values(const Real&)
  {
    return m_scalars;
  }

  VectorsT& values(const RealVector&)
  {
    return m_vectors;
  }

  /// Triggered when an option changes, copies all stored constants to RealVector storage
  void convert_vector_proxy()
  {

    for(VectorsProxiesT::const_iterator vec_it = m_vector_proxies.begin(); vec_it != m_vector_proxies.end(); ++vec_it)
    {
      const std::vector<Real>& vec_vals = vec_it->second;
      RealVector& real_vec = m_vectors[vec_it->first];
      const Uint nb_comps = vec_vals.size();
      real_vec.resize(nb_comps);
      for(Uint i = 0; i != nb_comps; ++i)
        real_vec[i] = vec_vals[i];
    }
  }

  DescriptionsT descriptions;
  ScalarsT m_scalars;
  VectorsT m_vectors;
  VectorsProxiesT m_vector_proxies;
};

/// Transform to replace an occurance of ConfigurableConstant with a reference to its value
struct ReplaceConfigurableConstant :
  boost::proto::transform< ReplaceConfigurableConstant >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {

    typedef typename boost::remove_reference<typename boost::proto::result_of::value<ExprT>::type>::type::value_type ValueT;

    typedef typename boost::proto::result_of::make_expr
    <
      boost::proto::tag::terminal,
      ValueT&
    >::type result_type;

    typedef typename ConstantStorageType<ValueT>::type ValuesT;

    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param values // state parameter
              , typename impl::data_param
    ) const
    {
      const ConfigurableConstant<ValueT>& constant = boost::proto::value(expr);
      values.descriptions[constant.name] = constant.description;
      std::pair<typename ValuesT::iterator, bool> insert_result = values.values(constant.default_value).insert(std::make_pair(constant.name, constant.default_value));

      return boost::proto::make_expr<boost::proto::tag::terminal>(boost::ref(insert_result.first->second));
    }
  };
};

/// Grammar replacing ConfigurableConstants in an expression.
struct ReplaceConfigurableConstants :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::terminal< ConfigurableConstant<boost::proto::_> >,
      ReplaceConfigurableConstant
    >,
    boost::proto::terminal<boost::proto::_>,
    boost::proto::nary_expr< boost::proto::_, boost::proto::vararg<ReplaceConfigurableConstants> >
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ConfigurableConstant_hpp
