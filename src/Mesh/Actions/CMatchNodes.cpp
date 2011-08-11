// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/StringConversion.hpp"
#include "Common/OptionArray.hpp"

#include "Mesh/Actions/CMatchNodes.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CNodeFaceCellConnectivity.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CMesh.hpp"
#include "Math/Functions.hpp"
#include "Math/Consts.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;
  using namespace Math::Functions;
  using namespace Math::Consts;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CMatchNodes, CMeshTransformer, LibActions> CMatchNodes_Builder;

//////////////////////////////////////////////////////////////////////////////

CMatchNodes::CMatchNodes( const std::string& name )
: CMeshTransformer(name)
{

  m_properties["brief"] = std::string("Match nodes in given regions");
  std::string desc;
  desc =
    "  Usage: CMatchNodes Regions:array[uri]=region1,region2\n\n";
  m_properties["description"] = desc;


  m_options.add_option< OptionArrayT<URI> >("Regions", std::vector<URI>())
      ->description("Regions to match nodes of");
}

/////////////////////////////////////////////////////////////////////////////

std::string CMatchNodes::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CMatchNodes::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" + properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CMatchNodes::execute()
{
  CMesh& mesh = *m_mesh.lock();

  std::map<std::size_t,Uint> hash_to_node_idx;

  CFinfo << mesh.tree() << CFendl;
  const Uint m_dim = mesh.geometry().coordinates().row_size();

  std::vector<URI> region_paths = option("Regions").value<std::vector<URI> >();

  CRegion& region_1 = *mesh.access_component_ptr(region_paths[0])->as_ptr<CRegion>();
  CRegion& region_2 = *mesh.access_component_ptr(region_paths[1])->as_ptr<CRegion>();
  CList<Uint>& used_nodes_region_1 = CEntities::used_nodes(region_1);
  CList<Uint>& used_nodes_region_2 = CEntities::used_nodes(region_2);

  // Check if regions have same number of used nodes
  if ( used_nodes_region_1.size() != used_nodes_region_2.size() )
    throw SetupError(FromHere(), "Number of used nodes in ["+region_1.uri().path()+"] and ["+region_2.uri().path()+"] are different.\n"
      "Nodes cannot be matched." );

  CTable<Real>& coordinates = mesh.geometry().coordinates();


  // find bounding box coordinates for region 1 and region 2
  enum {MIN=0,MAX=1};
  std::vector<RealVector3> bounding_1(2);
  std::vector<RealVector3> bounding_2(2);
  bounding_1[MIN].setConstant(real_max());    bounding_1[MAX].setConstant(real_min());
  bounding_2[MIN].setConstant(real_max());    bounding_2[MAX].setConstant(real_min());

  boost_foreach(const Uint coord_idx , used_nodes_region_1.array() )
  {
    CTable<Real>::ConstRow coords = coordinates[coord_idx];
    for (Uint d=0; d<m_dim; ++d)
    {
      bounding_1[MIN][d] = std::min(bounding_1[MIN][d],  coords[d]);
      bounding_1[MAX][d] = std::max(bounding_1[MAX][d],  coords[d]);
    }
  }


  boost_foreach(const Uint coord_idx , used_nodes_region_2.array() )
  {
    CTable<Real>::ConstRow coords = coordinates[coord_idx];
    for (Uint d=0; d<m_dim; ++d)
    {
      bounding_2[MIN][d] = std::min(bounding_2[MIN][d],  coords[d]);
      bounding_2[MAX][d] = std::max(bounding_2[MAX][d],  coords[d]);
    }
  }

  // Check 2 bounding boxes have same shape
  if ( hash_value(bounding_1[MAX] - bounding_1[MIN]) != hash_value(bounding_2[MAX] - bounding_2[MIN]) )
    throw SetupError(FromHere(), "Bounding boxes of ["+region_1.uri().path()+"] and ["+region_2.uri().path()+"] do not have the same shape.\n"
      "Nodes cannot be matched." );

  // put nodes of region 1 in a map with as key the
  // hash_value of the node coordinates - bounding_1[MIN]
  // and as value the index of the node in the coordinates table
  boost_foreach(const Uint coords_idx , used_nodes_region_1.array() )
  {
    CTable<Real>::ConstRow row = coordinates[coords_idx];
    RealVector3 coords; coords.setZero();
    for (Uint d=0; d<row.size(); ++d)
      coords[d] = row[d] - bounding_1[MIN][d];
    hash_to_node_idx[ hash_value(coords) ] = coords_idx;
  }


  std::map<std::size_t,Uint>::const_iterator not_found = hash_to_node_idx.end();
  boost_foreach(const Uint coords_idx , used_nodes_region_2.array() )
  {
    CTable<Real>::ConstRow row = coordinates[coords_idx];
    RealVector3 coords; coords.setZero();
    for (Uint d=0; d<row.size(); ++d)
      coords[d] = row[d] - bounding_2[MIN][d];

    std::map<std::size_t,Uint>::const_iterator it = hash_to_node_idx.find(hash_value(coords));
    if ( it != not_found )
    {
      const Uint matching_coords_idx = it->second;
      CFinfo << "match found: " << coords_idx << " <--> " << matching_coords_idx << CFendl;
    }
    else
    {
      throw SetupError(FromHere(), "1 or more nodes of ["+region_2.uri().path()+"] do not have a matching node in ["+region_1.uri().path()+"]." );
    }
  }
  CFinfo << "Node matching successful" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

std::size_t CMatchNodes::hash_value(const RealVector3& coords)
{
  std::size_t seed=0;
  // cast to float to round off numerical error leading to different hash values
  boost::hash_combine(seed,static_cast<float>(coords[0]));
  boost::hash_combine(seed,static_cast<float>(coords[1]));
  boost::hash_combine(seed,static_cast<float>(coords[2]));
  return seed;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
