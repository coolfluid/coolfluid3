// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_zoltan_PHG_hpp
#define CF_zoltan_PHG_hpp

#include "mesh/MeshTransformer.hpp"

#include "zoltan/LibZoltan.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace zoltan {

////////////////////////////////////////////////////////////////////////////////

/// Executes a mesh partitioning using the hypergraph (PHG) mechanism
class PHG : public mesh::MeshTransformer
{
public:
  PHG(const std::string& name);
  virtual ~PHG();
  
  static std::string type_name () { return "PHG"; }
  
  virtual void execute();
};
  
////////////////////////////////////////////////////////////////////////////////

} // zoltan
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF_zoltan_PHG_hpp
