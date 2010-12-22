// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_Proto_ElementVariables_hpp
#define CF_Actions_Proto_ElementVariables_hpp

#include <boost/proto/proto.hpp>

/// @file
/// Variables used in element-wise expressions

namespace CF {
namespace Actions {
namespace Proto {

/// Indicates the terminal represents a shape function member function that is global for the element, i.e. requires only information about the geometry
template<typename TagT>
struct SFGlobalFunction
{
};

/// Indicates the terminal represents a shape function member function that requires information about the
/// geometry (support) and mapped coordinates
template<typename TagT>
struct SFSupportFunction
{
};

/// Indicates the terminal represents a shape function member function that requires information about the
/// geometry (support), the field (states) and mapped coordinates
template<typename TagT>
struct SFFieldFunction
{
};

/// Define the different function tags
struct VolumeTag {};
struct JacobianTag {};
struct JacobianDeterminantTag {};
struct NormalTag {};
struct GradientTag {};
struct LaplacianTag {};
struct SFOuterProductTag {};

/// Static terminals that can be used in proto expressions
boost::proto::terminal< SFGlobalFunction<VolumeTag> >::type const volume = {{}};

boost::proto::terminal< SFSupportFunction<JacobianTag> >::type const jacobian = {{}};
boost::proto::terminal< SFSupportFunction<JacobianDeterminantTag> >::type const jacobian_determinant = {{}};
boost::proto::terminal< SFSupportFunction<NormalTag> >::type const normal = {{}};

boost::proto::terminal< SFFieldFunction<GradientTag> >::type const gradient = {{}};
boost::proto::terminal< SFFieldFunction<LaplacianTag> >::type const laplacian = {{}};

boost::proto::terminal< SFOuterProductTag >::type const sf_outer_product = {{}};

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
} // namespace CF

#endif // CF_Actions_Proto_ElementVariables_hpp
