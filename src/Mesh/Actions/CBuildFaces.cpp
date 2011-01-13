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

#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"

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
  //make_interfaces(m_mesh);
  build_inner_faces_bottom_up(m_mesh);
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
}

////////////////////////////////////////////////////////////////////////////////

void CBuildFaces::build_inner_faces_bottom_up(Component::Ptr parent)
{
  cf_assert_desc("parent must be a CRegion or CMesh", 
    is_not_null( parent->as_type<CMesh>() ) || is_not_null( parent->as_type<CRegion>() ) );
  
  boost_foreach( CRegion& region, find_components<CRegion>(*parent) )
  {
    build_inner_faces_bottom_up(region.self());
    
    if ( count( find_components<CElements>(region) ) != 0 )
    {
      // this region is the bottom region
      // CFaceCellConnectivity::Ptr face_to_cell = region.create_component<CFaceCellConnectivity>("face_to_cell");
      // build connectivity ==> 1) inner faces  2) bdry faces
      // 1) make regions for inner faces
      CRegion& inner_faces = region.create_region("inner_faces");
      // 2) get bdry elements from bdry faces and store in a set
    }
    else
    { 
      // this region is connected to another region
      make_interfaces(region.self());
      
      //CFaceCellConnectivity::Ptr face_to_cell = region.create_component<CFaceCellConnectivity>("face_to_cell");
      // build connectivity
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF
