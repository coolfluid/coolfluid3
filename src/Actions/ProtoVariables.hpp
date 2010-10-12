// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoVariables_hpp
#define CF_Actions_ProtoVariables_hpp

#include <boost/mpl/int.hpp>

#include "Actions/ProtoDomain.hpp"

namespace CF {
namespace Actions {

// Placeholders for different types of data
struct ElementIdxHolder {};
boost::proto::terminal<ElementIdxHolder>::type const _elem = {{}}; // Represents an element index ({{{}}} makes this a static initialization)

struct MappedCoordHolder {};
boost::proto::terminal<MappedCoordHolder>::type const _mapped_coord = {{}};

/// Creates a variable that has unique ID I
template<typename I, typename T>
struct Var : T
{
  /// Type that is wrapped
  typedef T type;
  Var() : T() {}
  
  /// Index of the var
  typedef I index_type;
  
  template<typename T1>
  Var(const T1& par1) : T(par1) {}
  
  /// Type of the terminal expression wrapping this variable
  typedef MeshExpr<typename boost::proto::terminal<Var<I, T> >::type> expr_type;
};

/// Represent const element nodes. This will be replaced with the current list of element nodes
struct ConstNodes
{
};

std::ostream& operator<<(std::ostream& output, const ConstNodes&)
{
  output << "ConstNodes";
  return output;
}

/// Const node field data
template<typename T>
struct ConstNodesFd
{
  ConstNodesFd() : field_name() {}
  
  ConstNodesFd(const std::string& fld_name) : field_name(fld_name) {}
  
  std::string field_name;
};

template<typename T>
std::ostream& operator<<(std::ostream& output, const ConstNodesFd<T>& fd)
{
  output << "field " << fd.field_name;
  return output;
}

/// Represent mutable element nodes
struct MutableNodes
{
};

template<Uint I, typename T>
struct MeshTerm : Var<boost::mpl::int_<I>, T>::expr_type
{
  typedef typename Var<boost::mpl::int_<I>, T>::expr_type base_type;
  
  MeshTerm() : base_type() {}
  
  template<typename T1>
  MeshTerm(const T1& par1) : base_type(par1) {}
};

} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoVariables_hpp
