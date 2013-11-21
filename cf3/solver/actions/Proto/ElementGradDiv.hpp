// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ElementGradDiv_hpp
#define cf3_solver_actions_Proto_ElementGradDiv_hpp

#include "ElementOperations.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Evaluate the gradient
struct GradientOp
{
  template<typename Signature>
  struct result;

  template<typename This, typename VarT, typename MappedCoordsT>
  struct result<This(VarT, MappedCoordsT)>
  {
    typedef const typename VarT::EtypeT::CoordsT& type;
  };

  template<typename This, typename VarT>
  struct result<This(VarT)>
  {
    typedef const typename VarT::EtypeT::CoordsT& type;
  };

  template<typename VarT, typename ResultT>
  const ResultT& operator()(ResultT& result, const VarT& var, const typename VarT::MappedCoordsT& mapped_coords) const
  {
    result.noalias() = var.nabla(mapped_coords) * var.value();
    return result;
  }

  template<typename VarT, typename ResultT>
  const ResultT& operator()(ResultT& result, const VarT& var) const
  {
    result.noalias() = var.nabla() * var.value();
    return result;
  }
};

/// Evaluate the divergence
struct DivOp
{
  // The result is a scalar
  typedef Real result_type;

  // Return the divergence of unknown var, computed at mapped_coords
  template<typename VarT>
  Real operator()(const VarT& var, const typename VarT::MappedCoordsT& mapped_coords) const
  {
    // Get the gradient matrix
    const typename VarT::GradientT& nabla = var.nabla(mapped_coords);
    Real result = 0.;
    // Apply each component and return the result
    for(int i = 0; i != VarT::EtypeT::dimensionality; ++i)
    {
      result += nabla.row(i) * var.value().col(i);
    }
    return result;
  }

  template<typename VarT>
  Real operator()(const VarT& var) const
  {
    const typename VarT::GradientT& nabla = var.nabla();
    Real result = 0.;
    for(int i = 0; i != VarT::EtypeT::dimensionality; ++i)
    {
      result += nabla.row(i) * var.value().col(i);
    }
    return result;
  }
};

/// Compute the gradient value of the given variable v:
/// grad(v): evaluation inside integral at the current quadrature point
/// grad(v, xi): evaluation at any mapped coordinate xi
static MakeSFOp<GradientOp>::type const gradient = {};

/// Compute the divergence of the given variable v:
/// div(v): evaluation inside integral at the current quadrature point
/// div(v, xi): evaluation at any mapped coordinate xi
static MakeSFOp<DivOp>::type const divergence = {};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ElementGradDiv_hpp
