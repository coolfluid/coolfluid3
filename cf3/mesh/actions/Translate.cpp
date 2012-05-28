// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StringConversion.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/actions/Translate.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  
////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Translate, MeshTransformer, mesh::actions::LibActions> Translate_Builder;

//////////////////////////////////////////////////////////////////////////////

Translate::Translate( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Translate mesh");
  std::string desc;
  desc =
    "  Usage: Translate vector:array[real]=dx,dy,dz\n\n";

  properties()["description"] = desc;

  std::vector<Real> vec = boost::assign::list_of(0.)(0.)(0.);
  options().add("vector",vec)
      .description("Translation vector")
      .mark_basic();
}

/////////////////////////////////////////////////////////////////////////////

void Translate::execute()
{
  // Get options
  std::vector<Real> vec = options().value< std::vector<Real> >("vector");
  const Uint dim = m_mesh->dimension();
  Uint d=0;

  boost_foreach(const Handle<Dictionary>& dict, m_mesh->dictionaries())
  {
    boost_foreach(Field::Row point, dict->coordinates().array())
    {
      for (d=0; d<dim; ++d)
        point[d] += vec[d];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
