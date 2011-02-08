// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/Actions/CBubbleEnrich.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"

#include "Math/MathFunctions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
  using namespace Math::MathFunctions;
  
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBubbleEnrich, CMeshTransformer, LibActions> CBubbleEnrich_Builder;

//////////////////////////////////////////////////////////////////////////////

CBubbleEnrich::CBubbleEnrich( const std::string& name )
: CMeshTransformer(name)
{
   
  properties()["brief"] = std::string("Enriches a Lagrangian space with bubble functions in each element");
  std::string desc = "  Usage: CBubbleEnrich \n\n";
	properties()["description"] = desc;
}

/////////////////////////////////////////////////////////////////////////////

std::string CBubbleEnrich::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CBubbleEnrich::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CBubbleEnrich::transform( const CMesh::Ptr& meshptr,
                               const std::vector<std::string>& args)
{

  CMesh& mesh = *meshptr;

  
}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF
