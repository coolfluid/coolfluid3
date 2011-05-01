// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"

#include "SFDM/Core/CreateSpace.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {
namespace Core {
  
  using namespace Common;
  using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CreateSpace, CMeshTransformer, LibCore> CreateSpace_Builder;

//////////////////////////////////////////////////////////////////////////////

CreateSpace::CreateSpace( const std::string& name )
: CMeshTransformer(name)
{
   
  properties()["brief"] = std::string("Create space for SFDM shape function");
  properties()["description"] = std::string("The polynomial order \"P\" is configurable, default: P = 0");

  properties().add_option( OptionT<Uint>::create("P","Polynomial Order","The order of the polynomial of the solution",0u) );
}
  
/////////////////////////////////////////////////////////////////////////////

void CreateSpace::execute()
{

  CMesh& mesh = *m_mesh.lock();

  boost_foreach(CEntities& entities, find_components_recursively_with_filter<CEntities>(mesh,IsElementsVolume()))
  {
    entities.create_space("CF.SFDM.SF."+entities.element_type().shape_name()+"SolutionP"+property("P").value_str());
    entities.create_space("CF.SFDM.SF."+entities.element_type().shape_name()+"FluxP"+to_str(property("P").value<Uint>()+1));
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // SFDM
} // CF
