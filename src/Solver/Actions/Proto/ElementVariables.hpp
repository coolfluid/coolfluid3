// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementVariables_hpp
#define CF_Solver_Actions_Proto_ElementVariables_hpp

#include <boost/proto/proto.hpp>

#include "Common/CF.hpp"

/// @file
/// Variables used in element-wise expressions

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Placeholder to indicate that a terminal is a shape function operation
template<typename OpT>
struct SFOp
{
};

/// Element volume
struct VolumeOp
{
  template<typename SupportT, typename MappedCoordsT, typename DataT>
  struct apply
  {
    typedef Real result_type;
    
    result_type operator()(SupportT& support, MappedCoordsT, DataT)
    {
      return support.volume();
    }
    
  };
};

/// Interpolated real-world coordinates at mapped coordinates
struct CoordinatesOp
{
  template<typename SupportT, typename MappedCoordsT, typename DataT>
  struct apply
  {
    typedef const typename SupportT::CoordsT& result_type;
    
    result_type operator()(SupportT& support, MappedCoordsT mapped_coords, DataT)
    {
      return support.coordinates(mapped_coords);
    }
  };
};

/// Interpolated values at mapped coordinates
struct InterpolationOp
{
  template<typename SupportT, typename MappedCoordsT, typename DataT>
  struct apply
  {
    typedef typename boost::remove_reference<DataT>::type::EvalT result_type;
    
    result_type operator()(SupportT&, MappedCoordsT mapped_coords, DataT data)
    {
      return data.eval(mapped_coords);
    }
  };
};

/// Jacobian matrix
struct JacobianOp
{
  template<typename SupportT, typename MappedCoordsT, typename DataT>
  struct apply
  {
    typedef const typename SupportT::ShapeFunctionT::JacobianT& result_type;
    
    result_type operator()(SupportT& support, MappedCoordsT mapped_coords, DataT)
    {
      return support.jacobian(mapped_coords);
    }
  };
};

/// Jacobian determinant
struct JacobianDeterminantOp
{
  template<typename SupportT, typename MappedCoordsT, typename DataT>
  struct apply
  {
    typedef Real result_type;
    
    result_type operator()(SupportT& support, MappedCoordsT mapped_coords, DataT)
    {
      return support.jacobian_determinant(mapped_coords);
    }
  };
};

/// Face Normal
struct NormalOp
{
  template<typename SupportT, typename MappedCoordsT, typename DataT>
  struct apply
  {
    typedef const typename SupportT::ShapeFunctionT::CoordsT& result_type;
    
    result_type operator()(SupportT& support, MappedCoordsT mapped_coords, DataT)
    {
      return support.normal(mapped_coords);
    }
  };
};

/// Gradient
struct GradientOp
{
  template<typename SupportT, typename MappedCoordsT, typename DataT>
  struct apply
  {
    typedef const typename SupportT::ShapeFunctionT::MappedGradientT& result_type;
    
    result_type operator()(SupportT& support, MappedCoordsT mapped_coords, DataT data)
    {
      return data.gradient(mapped_coords, support);
    }
  };
};

/// Laplacian
struct LaplacianOp
{
  template<typename SupportT, typename MappedCoordsT, typename DataT>
  struct apply
  {
    typedef const typename SupportT::LaplacianT& result_type;
    
    result_type operator()(SupportT& support, MappedCoordsT mapped_coords, DataT data)
    {
      return data.laplacian(mapped_coords, support);
    }
  };
};

/// Outer product
struct OuterProductOp
{
  template<typename SupportT, typename MappedCoordsT, typename DataT>
  struct apply
  {
    typedef const typename SupportT::LaplacianT& result_type;
    
    result_type operator()(SupportT& support, MappedCoordsT mapped_coords, DataT data)
    {
      return data.sf_outer_product(mapped_coords);
    }
  };
};

/// Static terminals that can be used in proto expressions
boost::proto::terminal< SFOp<VolumeOp> >::type const volume = {{}};

boost::proto::terminal< SFOp<CoordinatesOp> >::type const coordinates = {{}};
boost::proto::terminal< SFOp<JacobianOp> >::type const jacobian = {{}};
boost::proto::terminal< SFOp<JacobianDeterminantOp> >::type const jacobian_determinant = {{}};
boost::proto::terminal< SFOp<NormalOp> >::type const normal = {{}};

boost::proto::terminal< SFOp<GradientOp> >::type const gradient = {{}};
boost::proto::terminal< SFOp<LaplacianOp> >::type const laplacian = {{}};

boost::proto::terminal< SFOp<OuterProductOp> >::type const sf_outer_product = {{}};

/// Tag for an integral, wit the order provided as an MPL integral constant
template<typename OrderT>
struct IntegralTag
{
};

/// Placeholder for integration
template<Uint Order, typename ArgT>
typename boost::proto::function
<
  typename boost::proto::terminal
  <
    IntegralTag< boost::mpl::int_<Order> >
  >::type,
  ArgT
>::type
integral(ArgT const &arg)
{
  typedef typename boost::proto::function
  <
    typename boost::proto::terminal
    <
      IntegralTag< boost::mpl::int_<Order> >
    >::type,
    ArgT
  >::type result_type;

  result_type result = {{{}}, arg};
  return result;
}

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementVariables_hpp
