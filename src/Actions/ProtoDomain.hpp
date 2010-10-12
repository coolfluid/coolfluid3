// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoDomain_hpp
#define CF_Actions_ProtoDomain_hpp

#include <boost/fusion/sequence/intrinsic/value_at.hpp>

#include "Actions/ProtoGrammars.hpp"

namespace CF {
namespace Actions {

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
  
  /// Constructor taking a single parameter
  template<typename T>
  MeshExpr(const T& par1)
  {
    typedef typename boost::fusion::result_of::value_at_c<Expr, 0>::type var_t;
    boost::proto::value(*this) = var_t(par1);
  }
};

} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoDomain_hpp
