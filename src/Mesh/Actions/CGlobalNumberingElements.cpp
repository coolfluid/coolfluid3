// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#define BOOST_HASH_NO_IMPLICIT_CASTS
#include <boost/functional/hash.hpp>

#include <boost/static_assert.hpp>
#include <set>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/StringConversion.hpp"
#include "Common/OptionArray.hpp"
#include "Common/CreateComponentDataType.hpp"
#include "Common/OptionT.hpp"
#include "Common/PE/Comm.hpp"
#include "Common/PE/debug.hpp"

#include "Mesh/Actions/CGlobalNumberingElements.hpp"
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
#include "Mesh/ElementData.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;
  using namespace Math::Functions;
  using namespace Math::Consts;

  create_component_data_type( std::vector<std::size_t> , Mesh_Actions_API , CVector_size_t , "CVector<size_t>" );

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CGlobalNumberingElements, CMeshTransformer, LibActions> CGlobalNumberingElements_Builder;

//////////////////////////////////////////////////////////////////////////////

CGlobalNumberingElements::CGlobalNumberingElements( const std::string& name )
: CMeshTransformer(name),
  m_debug(false)
{

  m_properties["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc =
    "  Usage: CGlobalNumberingElements Regions:array[uri]=region1,region2\n\n";
  m_properties["description"] = desc;

  m_options.add_option<OptionT<bool> >("debug", m_debug)
      ->description("Perform checks on validity")
      ->pretty_name("Debug")
      ->link_to(&m_debug);

  m_options.add_option<OptionT<bool> >("combined", true)
      ->description("Combine nodes and elements in one global numbering")
      ->pretty_name("Combined");
}

/////////////////////////////////////////////////////////////////////////////

std::string CGlobalNumberingElements::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CGlobalNumberingElements::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" + properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CGlobalNumberingElements::execute()
{
  CMesh& mesh = *m_mesh.lock();

  CTable<Real>& coordinates = mesh.geometry().coordinates();


  boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
  {
    RealMatrix element_coordinates(elements.element_type().nb_nodes(),coordinates.row_size());

    if ( is_null( elements.get_child_ptr("glb_elem_hash") ) )
      elements.create_component<CVector_size_t>("glb_elem_hash");
    CVector_size_t& glb_elem_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>();
    glb_elem_hash.data().resize(elements.size());

    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      elements.put_coordinates(element_coordinates,elem_idx);
      glb_elem_hash.data()[elem_idx]=hash_value(element_coordinates);
      if (m_debug)
        std::cout << "["<<PE::Comm::instance().rank() << "]  hashing elem ("<< elements.uri().path() << "["<<elem_idx<<"]) to " << glb_elem_hash.data()[elem_idx] << std::endl;

      //CFinfo << "glb_elem_hash["<<elem_idx<<"] = " <<  glb_elem_hash.data()[elem_idx] << CFendl;
    }
  }


  // In debug mode, check if no hashes are duplicated
  if (m_debug)
  {
    std::set<std::size_t> glb_set;

    boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
    {
      CVector_size_t& glb_elem_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>();
      for (Uint i=0; i<glb_elem_hash.data().size(); ++i)
      {
        if (glb_set.insert(glb_elem_hash.data()[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] is duplicated");
      }
    }

  }


  // now renumber

  //------------------------------------------------------------------------------
  // get tot nb of owned indexes and communicate

  Uint tot_nb_owned_ids=0;
  boost_foreach( CEntities& elements, find_components_recursively<CElements>(mesh) )
    tot_nb_owned_ids += elements.size();

  std::vector<Uint> nb_ids_per_proc(PE::Comm::instance().size());
  //boost::MPI::communicator world;
  //boost::MPI::all_gather(world, tot_nb_owned_ids, nb_ids_per_proc);
  PE::Comm::instance().all_gather(tot_nb_owned_ids, nb_ids_per_proc);
  std::vector<Uint> start_id_per_proc(PE::Comm::instance().size());
  Uint start_id=0;
  for (Uint p=0; p<nb_ids_per_proc.size(); ++p)
  {
    start_id_per_proc[p] = start_id;
    start_id += nb_ids_per_proc[p];
  }



  //------------------------------------------------------------------------------
  // give glb idx to elements
  Uint glb_id=start_id_per_proc[PE::Comm::instance().rank()];
  boost_foreach( CEntities& elements, find_components_recursively<CElements>(mesh) )
  {
    CList<Uint>& elements_glb_idx = elements.glb_idx();
    elements_glb_idx.resize(elements.size());
    std::vector<std::size_t>& glb_elem_hash = elements.get_child("glb_elem_hash").as_type<CVector_size_t>().data();
    cf_assert(glb_elem_hash.size() == elements.size());
    for (Uint e=0; e<elements.size(); ++e)
    {
      if (m_debug)
        std::cout << "["<<PE::Comm::instance().rank() << "]  will change elem "<< glb_elem_hash[e] << " (" << elements.uri().path() << "["<<e<<"]) to " << glb_id << std::endl;
      elements_glb_idx[e] = glb_id;
      ++glb_id;
    }
  }


  // In debug mode, check if no hashes are duplicated
  if (m_debug)
  {
    std::set<Uint> glb_set;

    boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
    {
      CList<Uint>& elements_glb_idx = elements.glb_idx();
      for (Uint i=0; i<elements.size(); ++i)
      {
        if (glb_set.insert(elements_glb_idx[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] is duplicated");
      }
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

std::size_t CGlobalNumberingElements::hash_value(const RealMatrix& coords)
{
  std::size_t seed=0;
  for (Uint i=0; i<coords.rows(); ++i)
  for (Uint j=0; j<coords.cols(); ++j)
    boost::hash_combine(seed,coords(i,j));
  return seed;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
