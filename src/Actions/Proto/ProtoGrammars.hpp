// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoGrammars_hpp
#define CF_Actions_ProtoGrammars_hpp

#include <boost/proto/proto.hpp>

#include "Common/CF.hpp"

namespace CF {
namespace Actions {

//////////////////////////////////////////////////////////
// Grammar that must be matched for an expression to be valid

// struct MeshGrammar
//   : proto::or_<
//         proto::terminal< Real >
//       , proto::plus< MeshGrammar, MeshGrammar >
//       , proto::minus< MeshGrammar, MeshGrammar >
//       , proto::multiplies< MeshGrammar, MeshGrammar >
//       , proto::plus_assign< MeshGrammar, MeshGrammar >
//       , proto::shift_left< proto::terminal< std::ostream & >, proto::_ >
//       , proto::shift_left< MeshGrammar, proto::_ >
//       , proto::subscript< proto::_, proto::_ > // TODO: deal with subscript properly
//     >
// {};

/// Todo: This grammar should only match expressions that are valid on a mesh.
/// For now, we accept anything, since the commented attempt above is incomplete
struct MeshGrammar : boost::proto::_ {};

} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoGrammars_hpp
