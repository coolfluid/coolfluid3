// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoDomain_hpp
#define CF_Actions_ProtoDomain_hpp

#include <boost/fusion/sequence/intrinsic/value_at.hpp>

#include "Actions/Proto/ProtoGrammars.hpp"

namespace CF {
namespace Actions {
namespace Proto {

/// Wrapper for expressions in the MeshDomain
template<class Expr>
struct MeshExpr;

/// The domain for expressions related to mesh operations
struct MeshDomain
  : boost::proto::domain<boost::proto::generator<MeshExpr>, MeshGrammar>
{};

template<class Expr>
struct MeshExpr : boost::proto::extends<Expr, MeshExpr<Expr>, MeshDomain>
{
  typedef boost::proto::extends<Expr, MeshExpr<Expr>, MeshDomain> base_type;

  MeshExpr(Expr const &expr = Expr()) : base_type(expr) {}
  
  // Overrides the compiler-generated operator=
  BOOST_PROTO_EXTENDS_USING_ASSIGN(MeshExpr)
};

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoDomain_hpp
