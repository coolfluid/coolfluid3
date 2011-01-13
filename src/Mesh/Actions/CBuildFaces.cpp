// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"

#include "Math/MathFunctions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
  using namespace Math::MathFunctions;
  
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBuildFaces, CMeshTransformer, LibActions> CBuildFaces_Builder;

//////////////////////////////////////////////////////////////////////////////

CBuildFaces::CBuildFaces( const std::string& name )
: CMeshTransformer(name)
{
   
	properties()["brief"] = std::string("Print information of the mesh");
	std::string desc;
	desc = 
  "  Usage: Info \n\n"
  "          Information given: internal mesh hierarchy,\n"
  "      element distribution for each region, and element type"; 
	properties()["description"] = desc;
}

/////////////////////////////////////////////////////////////////////////////

std::string CBuildFaces::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CBuildFaces::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CBuildFaces::transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args)
{

  m_mesh = mesh;

  // traverse regions and make interface region between connected regions recursively
  make_interfaces(m_mesh);
}

//////////////////////////////////////////////////////////////////////////////

void CBuildFaces::make_interfaces(Component::Ptr parent)
{
  cf_assert_desc("parent must be a CRegion or CMesh", 
    is_not_null( parent->as_type<CMesh>() ) || is_not_null( parent->as_type<CRegion>() ) );
  
  std::vector<CRegion::Ptr> regions = range_to_vector(find_components<CRegion>(*parent));
  const Uint n=regions.size();
  if (n>1)
  {
    Uint nb_interfaces = factorial(n) / (2*factorial(n-2));
    for (Uint i=0; i<n; ++i)
    for (Uint j=i+1; j<n; ++j)
    {
      CRegion& interface = *parent->create_component<CRegion>("interface_"+regions[i]->name()+"_to_"+regions[j]->name());
      interface.add_tag("interface");
    }
  }
  
  for (Uint i=0; i<n; ++i)
    make_interfaces(regions[i]); 
}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF
