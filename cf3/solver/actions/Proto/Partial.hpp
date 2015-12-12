// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_Partial_hpp
#define cf3_solver_actions_Proto_Partial_hpp

#include "IndexLooping.hpp"
#include "Terminals.hpp"
#include "Transforms.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

struct PartialOp : boost::proto::callable
{
  template<typename Signature>
  struct result;

  template<typename This, typename... ArgsT>
  struct result<This(ArgsT...)>
  {
    typedef decltype(std::declval<This>()(std::declval<ArgsT>()...)) type;
  };

  // Repeated index -> return the divergence
  template<typename VarT, typename IdxT>
  Real operator()(const VarT& var, IdxT, IdxT) const
  {
    static_assert(VarT::dimension == VarT::EtypeT::dimension, "Divergence must be computed on vectors");
    const typename VarT::GradientT& nabla = var.nabla();
    Real result = 0.;
    for(int i = 0; i != VarT::EtypeT::dimensionality; ++i)
    {
      result += nabla.row(i) * var.value().col(i);
    }
    return result;
  }

  template<typename VarT, typename IdxT>
  Real operator()(const VarT& var, IdxT, IdxT, const typename VarT::MappedCoordsT& coords) const
  {
    var.compute_values(coords);
    return (*this)(var, IdxT(), IdxT());
  }

  // Gradient matrix for vector field
  template<typename VarT>
  auto operator()(const VarT& var, I_t, J_t) const -> decltype(var.field_gradient())
  {
    static_assert(VarT::dimension == VarT::EtypeT::dimension, "partial(v[_i], _j) is for vectors");
    return var.field_gradient();
  }

  template<typename VarT>
  auto operator()(const VarT& var, I_t, J_t, const typename VarT::MappedCoordsT& coords) const -> decltype(var.field_gradient())
  {
    static_assert(VarT::dimension == VarT::EtypeT::dimension, "partial(v[_i], _j, coord) is for vectors");
    return var.field_gradient(coords);
  }

  // Transposed gradient matrix for vector field
  template<typename VarT>
  auto operator()(const VarT& var, J_t, I_t) const -> decltype(var.field_gradient().transpose())
  {
    static_assert(VarT::dimension == VarT::EtypeT::dimension, "partial(v[_j], _i) is for vectors");
    return var.field_gradient().transpose();
  }

  template<typename VarT>
  auto operator()(const VarT& var, J_t, I_t, const typename VarT::MappedCoordsT& coords) const -> decltype(var.field_gradient().transpose())
  {
    static_assert(VarT::dimension == VarT::EtypeT::dimension, "partial(v[_j], _i, coord) is for vectors");
    return var.field_gradient(coords).transpose();
  }

  // Gradient vector for scalar field
  template<typename VarT>
  auto operator()(const VarT& var, I_t) const -> decltype(var.field_gradient())
  {
    static_assert(VarT::dimension == 1, "partial(phi, _i) is for scalars");
    return var.field_gradient();
  }

  template<typename VarT>
  auto operator()(const VarT& var, I_t, const typename VarT::MappedCoordsT& coords) const -> decltype(var.field_gradient())
  {
    static_assert(VarT::dimension == 1, "partial(phi, _i, coord) is for scalars");
    return var.field_gradient(coords);
  }

  // Gradient vector transpose for scalar field
  template<typename VarT>
  auto operator()(const VarT& var, J_t) const -> decltype(var.field_gradient().transpose())
  {
    static_assert(VarT::dimension == 1, "partial(phi, _j) is for scalars");
    return var.field_gradient().transpose();
  }

  template<typename VarT>
  auto operator()(const VarT& var, J_t, const typename VarT::MappedCoordsT& coords) const -> decltype(var.field_gradient().transpose())
  {
    static_assert(VarT::dimension == 1, "partial(phi, _j, coord) is for scalars");
    return var.field_gradient(coords).transpose();
  }
};

struct PartialGrammar :
  boost::proto::or_
  <
    boost::proto::when // Variant without mapped coordinates as last argument
    <
      boost::proto::function // partial(u[_i], _j)
      <
        boost::proto::terminal<PartialTag>,
        boost::proto::subscript<FieldTypes, boost::proto::terminal<IndexTag<boost::proto::_>>>,
        boost::proto::terminal<IndexTag<boost::proto::_>>
      >,
      PartialOp
      (
        VarData(boost::proto::_value(boost::proto::_child0(boost::proto::_child1))),
        boost::proto::_value(boost::proto::_child1(boost::proto::_child1)),
        boost::proto::_value(boost::proto::_child2)
      )
    >,
    boost::proto::when // Variant with mapped coordinates as last argument
    <
      boost::proto::function // partial(u[_i], _j, coord)
      <
        boost::proto::terminal<PartialTag>,
        boost::proto::subscript<FieldTypes, boost::proto::terminal<IndexTag<boost::proto::_>>>,
        boost::proto::terminal<IndexTag<boost::proto::_>>,
        boost::proto::terminal<boost::proto::_>
      >,
      PartialOp
      (
        VarData(boost::proto::_value(boost::proto::_child0(boost::proto::_child1))),
        boost::proto::_value(boost::proto::_child1(boost::proto::_child1)),
        boost::proto::_value(boost::proto::_child2),
        boost::proto::_value(boost::proto::_child3)
      )
    >,
    boost::proto::when // Variant without mapped coordinates as last argument
    <
      boost::proto::function // partial(phi, _i)
      <
        boost::proto::terminal<PartialTag>,
        FieldTypes,
        boost::proto::terminal<IndexTag<boost::proto::_>>
      >,
      PartialOp
      (
        VarData(boost::proto::_value(boost::proto::_child1)),
        boost::proto::_value(boost::proto::_child2)
      )
    >,
    boost::proto::when // Variant with mapped coordinates as last argument
    <
      boost::proto::function // partial(phi, _i, coord)
      <
        boost::proto::terminal<PartialTag>,
        FieldTypes,
        boost::proto::terminal<IndexTag<boost::proto::_>>,
        boost::proto::terminal<boost::proto::_>
      >,
      PartialOp
      (
        VarData(boost::proto::_value(boost::proto::_child1)),
        boost::proto::_value(boost::proto::_child2),
        boost::proto::_value(boost::proto::_child3)
      )
    >
  >
{
};


} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_Partial_hpp
