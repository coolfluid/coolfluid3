// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/static_assert.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/StringConversion.hpp"
#include "Common/OptionArray.hpp"
#include "Common/CreateComponentDataType.hpp"

#include "Mesh/Actions/CGlobalNumbering.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CNodeFaceCellConnectivity.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CMesh.hpp"
#include "Math/MathFunctions.hpp"
#include "Math/MathConsts.hpp"
#include "Mesh/ElementData.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
  using namespace Math::MathFunctions;
  using namespace Math::MathConsts;
  
  create_component_data_type( std::vector<std::size_t> , Mesh_Actions_API , CVector_size_t , "CVector<size_t>" );
    
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CGlobalNumbering, CMeshTransformer, LibActions> CGlobalNumbering_Builder;

//////////////////////////////////////////////////////////////////////////////

CGlobalNumbering::CGlobalNumbering( const std::string& name )
: CMeshTransformer(name)
{
   
  properties()["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc = 
    "  Usage: CGlobalNumbering Regions:array[uri]=region1,region2\n\n";
  properties()["description"] = desc;
  
}

/////////////////////////////////////////////////////////////////////////////

std::string CGlobalNumbering::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CGlobalNumbering::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CGlobalNumbering::execute()
{
  CMesh& mesh = *m_mesh.lock();

  CTable<Real>& coordinates = mesh.nodes().coordinates();
  CVector_size_t& glb_node_hash = *mesh.nodes().create_component<CVector_size_t>("glb_node_hash");
  glb_node_hash.data().resize(coordinates.size());
  Uint i(0);
  boost_foreach(CTable<Real>::ConstRow coords, coordinates.array() )
  {
    glb_node_hash.data()[i]=hash_value(to_vector(coords));
    CFinfo << "glb_node_hash["<<i<<"] = " << glb_node_hash.data()[i] << CFendl;
    ++i;
  }
  
  boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
  {
    RealMatrix element_coordinates(elements.element_type().nb_nodes(),coordinates.row_size());
    CVector_size_t& glb_elem_hash = *elements.create_component<CVector_size_t>("glb_elem_hash");
    glb_elem_hash.data().resize(elements.size());
    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      elements.put_coordinates(element_coordinates,elem_idx);
      glb_elem_hash.data()[elem_idx]=hash_value(element_coordinates);
      CFinfo << "glb_elem_hash["<<elem_idx<<"] = " <<  glb_elem_hash.data()[elem_idx] << CFendl;
    }
  }
  CFinfo << mesh.tree() << CFendl;
  CFinfo << "Global Numbering successful" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

std::size_t CGlobalNumbering::hash_value(const RealVector& coords)
{
  std::size_t seed=0;
  // cast to float to round off numerical error leading to different hash values
  for (Uint i=0; i<coords.size(); ++i)
    boost::hash_combine(seed,static_cast<float>(coords[i]));
  return seed;
}

////////////////////////////////////////////////////////////////////////////////

std::size_t CGlobalNumbering::hash_value(const RealMatrix& coords)
{
  std::size_t seed=0;
  // cast to float to round off numerical error leading to different hash values
  for (Uint i=0; i<coords.rows(); ++i)
  for (Uint j=0; j<coords.cols(); ++j)
    boost::hash_combine(seed,static_cast<float>(coords(i,j)));
  return seed;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
