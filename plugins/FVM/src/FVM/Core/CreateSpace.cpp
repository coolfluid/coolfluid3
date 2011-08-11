// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "FVM/Core/CreateSpace.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace FVM {
namespace Core {
  
  using namespace Common;
  using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CreateSpace, CMeshTransformer, LibCore> CreateSpace_Builder;

//////////////////////////////////////////////////////////////////////////////

CreateSpace::CreateSpace( const std::string& name )
: CMeshTransformer(name)
{
   
  properties()["brief"] = std::string("Create space for FVM shape function");
  properties()["description"] = std::string("The polynomial order \"P\" is configurable, default: P = 0");
}
  
/////////////////////////////////////////////////////////////////////////////

void CreateSpace::execute()
{

  CMesh& mesh = *m_mesh.lock();

  boost_foreach(CEntities& entities, find_components_recursively_with_filter<CEntities>(mesh,IsElementsVolume()))
  {
    entities.create_space("P0","CF.Mesh.SF.SF"+entities.element_type().shape_name()+"Lagrange"+option("P0").value_str());
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF
