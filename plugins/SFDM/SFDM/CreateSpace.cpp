// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"

#include "SFDM/CreateSpace.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

  using namespace common;
  using namespace mesh;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CreateSpace, MeshTransformer, LibSFDM> CreateSpace_Builder;

//////////////////////////////////////////////////////////////////////////////

CreateSpace::CreateSpace( const std::string& name )
: MeshTransformer(name)
{

  m_properties["brief"] = std::string("Create space for SFDM shape function");
  m_properties["description"] = std::string("The polynomial order \"P\" of the solution is configurable, default: P = 0");

  options().add_option( OptionT<Uint>::create("P", 0u) )
    .description("The order of the polynomial of the solution")
    .pretty_name("Polynomial Order");
}

/////////////////////////////////////////////////////////////////////////////

void CreateSpace::execute()
{

  Mesh& mesh = *m_mesh;

  Uint p = option("P").value<Uint>();
  boost_foreach(Entities& entities, find_components_recursively_with_filter<Entities>(mesh,IsElementsVolume()))
  {
    entities.create_space("solution","cf3.SFDM.SF."+entities.element_type().shape_name()+"SolutionP"+to_str(p));
    entities.create_space("flux",    "cf3.SFDM.SF."+entities.element_type().shape_name()+"FluxP"    +to_str(p+1));
    //CFinfo << "local coords ("<< entities.space(1).shape_function().local_coordinates().rows()<<"x"<< entities.space(1).shape_function().local_coordinates().cols()<< ") = " << entities.space(1).shape_function().local_coordinates() << CFendl;
  }
}

//////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF
