// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"

#include "SFDM/CreateSpace.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {

  using namespace Common;
  using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CreateSpace, CMeshTransformer, LibSFDM> CreateSpace_Builder;

//////////////////////////////////////////////////////////////////////////////

CreateSpace::CreateSpace( const std::string& name )
: CMeshTransformer(name)
{

  m_properties["brief"] = std::string("Create space for SFDM shape function");
  m_properties["description"] = std::string("The polynomial order \"P\" of the solution is configurable, default: P = 0");

  m_options.add_option( OptionT<Uint>::create("P","Polynomial Order","The order of the polynomial of the solution",0u) );
}

/////////////////////////////////////////////////////////////////////////////

void CreateSpace::execute()
{

  CMesh& mesh = *m_mesh.lock();

  Uint p = option("P").value<Uint>();
  boost_foreach(CEntities& entities, find_components_recursively_with_filter<CEntities>(mesh,IsElementsVolume()))
  {
    entities.create_space("solution","CF.SFDM.SF."+entities.element_type().shape_name()+"SolutionP"+to_str(p));
    entities.create_space("flux",    "CF.SFDM.SF."+entities.element_type().shape_name()+"FluxP"    +to_str(p+1));
    //CFinfo << "local coords ("<< entities.space(1).shape_function().local_coordinates().rows()<<"x"<< entities.space(1).shape_function().local_coordinates().cols()<< ") = " << entities.space(1).shape_function().local_coordinates() << CFendl;
  }
}

//////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF
