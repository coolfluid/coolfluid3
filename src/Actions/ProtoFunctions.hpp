// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoFunctions_hpp
#define CF_Actions_ProtoFunctions_hpp

#include <cmath>

#include "Actions/ProtoDomain.hpp"

namespace CF {
namespace Actions {

/// Wrap the jacobian determinant function
struct jacobian_determinant_tag
{};

template<typename MappedCoordsT, typename NodesT>
typename boost::proto::result_of::make_expr<
    jacobian_determinant_tag
  , MeshDomain
  , MappedCoordsT const &
  , NodesT const &
>::type const
jacobian_determinant(MappedCoordsT const& mapped_coords, NodesT const& nodes)
{
  return boost::proto::make_expr<jacobian_determinant_tag, MeshDomain>(boost::ref(mapped_coords), boost::ref(nodes));
}

/// Return the value of the normal at the given mapped coordinates for an element with the given nodes
// Wrap the normal function
struct normal_tag
{};

template<typename MappedCoordsT, typename NodesT>
typename boost::proto::result_of::make_expr<
    normal_tag
  , MeshDomain
  , MappedCoordsT const &
  , NodesT const &
>::type const
normal(MappedCoordsT const& mapped_coords, NodesT const& nodes)
{
  return boost::proto::make_expr<normal_tag, MeshDomain>(boost::ref(mapped_coords), boost::ref(nodes));
}

/// Return element volume
struct volume_tag
{};

template<typename Arg>
typename boost::proto::result_of::make_expr<
    volume_tag
  , MeshDomain
  , Arg const &
>::type const
volume(Arg const &arg)
{
  return boost::proto::make_expr<volume_tag, MeshDomain>(boost::ref(arg));
}

/// Integrates the argument
template<Uint Order>
struct integral_tag
{};

template<Uint Order, typename Arg>
typename boost::proto::result_of::make_expr<
    integral_tag<Order>
  , MeshDomain
  , Arg const &
>::type const
integral(Arg const &arg)
{
  return boost::proto::make_expr<integral_tag<Order>, MeshDomain>(boost::ref(arg));
}

/// Convert mapped coordinates to real coordinates
struct coords_tag
{};

template<typename MappedCoordsT, typename NodesT>
typename boost::proto::result_of::make_expr<
    coords_tag
  , MeshDomain
  , MappedCoordsT const &
  , NodesT const &
>::type const
coords(MappedCoordsT const& mapped_coords, NodesT const& nodes)
{
  return boost::proto::make_expr<coords_tag, MeshDomain>(boost::ref(mapped_coords), boost::ref(nodes));
}

/// Pow function based on Proto docs example
template<Uint Exp>
struct pow_fun
{
  typedef Real result_type;
  Real operator()(Real d) const
  {
    for(Uint i = 0; i != (Exp-1); ++i)
      d *= d;
    return d;
  }
};

template<Uint Exp, typename Arg>
typename boost::proto::result_of::make_expr<
    boost::proto::tag::function  // Tag type
  , pow_fun<Exp>          // First child (by value)
  , Arg const &           // Second child (by reference)
>::type const
pow(Arg const &arg)
{
  return boost::proto::make_expr<boost::proto::tag::function>(
      pow_fun<Exp>()    // First child (by value)
    , boost::ref(arg)   // Second child (by reference)
  );
}

// Wrap some math functions
boost::proto::terminal< double(*)(double) >::type const _sin = {&sin};
boost::proto::terminal< double(*)(double, double) >::type const _atan2 = {&atan2};

// Wrap std::cout
boost::proto::terminal< std::ostream & >::type _cout = {std::cout};

} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoFunctions_hpp
