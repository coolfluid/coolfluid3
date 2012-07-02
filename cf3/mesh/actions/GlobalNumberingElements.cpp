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

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StreamHelpers.hpp"
#include "common/StringConversion.hpp"
#include "common/OptionArray.hpp"
#include "common/CreateComponentDataType.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "mesh/actions/GlobalNumberingElements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/NodeElementConnectivity.hpp"
#include "mesh/Node2FaceCellConnectivity.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "math/Functions.hpp"
#include "math/Consts.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Field.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace math::Functions;
  using namespace math::Consts;

  create_component_data_type( std::vector<std::size_t> , mesh_actions_API , CVector_size_t , "CVector<size_t>" );

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < GlobalNumberingElements, MeshTransformer, mesh::actions::LibActions> GlobalNumberingElements_Builder;

//////////////////////////////////////////////////////////////////////////////

GlobalNumberingElements::GlobalNumberingElements( const std::string& name )
: MeshTransformer(name),
  m_debug(false)
{

  properties()["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc =
    "  Usage: GlobalNumberingElements Regions:array[uri]=region1,region2\n\n";
  properties()["description"] = desc;

  options().add("debug", m_debug)
      .description("Perform checks on validity")
      .pretty_name("Debug")
      .link_to(&m_debug);

  options().add("combined", true)
      .description("Combine nodes and elements in one global numbering")
      .pretty_name("Combined");
}

/////////////////////////////////////////////////////////////////////////////

void GlobalNumberingElements::execute()
{
  Mesh& mesh = *m_mesh;

  common::Table<Real>& coordinates = mesh.geometry_fields().coordinates();


  boost_foreach( Elements& elements, find_components_recursively<Elements>(mesh) )
  {
    RealMatrix element_coordinates;
    elements.geometry_space().allocate_coordinates(element_coordinates);

    if ( is_null( elements.get_child("glb_elem_hash") ) )
      elements.create_component<CVector_size_t>("glb_elem_hash");
    CVector_size_t& glb_elem_hash = *Handle<CVector_size_t>(elements.get_child("glb_elem_hash"));
    glb_elem_hash.data().resize(elements.size());

    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      elements.geometry_space().put_coordinates(element_coordinates,elem_idx);
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

    boost_foreach( Elements& elements, find_components_recursively<Elements>(mesh) )
    {
      CVector_size_t& glb_elem_hash = *Handle<CVector_size_t>(elements.get_child("glb_elem_hash"));
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
  boost_foreach( Entities& elements, find_components_recursively<Elements>(mesh) )
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
  boost_foreach( Entities& elements, find_components_recursively<Elements>(mesh) )
  {
    common::List<Uint>& elements_glb_idx = elements.glb_idx();
    elements_glb_idx.resize(elements.size());
    std::vector<std::size_t>& glb_elem_hash = Handle<CVector_size_t>(elements.get_child("glb_elem_hash"))->data();
    cf3_assert(glb_elem_hash.size() == elements.size());
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

    boost_foreach( Elements& elements, find_components_recursively<Elements>(mesh) )
    {
      common::List<Uint>& elements_glb_idx = elements.glb_idx();
      for (Uint i=0; i<elements.size(); ++i)
      {
        if (glb_set.insert(elements_glb_idx[i]).second == false)  // it was already in the set
          throw ValueExists(FromHere(), "elem "+elements.uri().path()+"["+to_str(i)+"] is duplicated");
      }
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

std::size_t GlobalNumberingElements::hash_value(const RealMatrix& coords)
{
  std::size_t seed=0;
  for (Uint i=0; i<coords.rows(); ++i)
  for (Uint j=0; j<coords.cols(); ++j)
    boost::hash_combine(seed,coords(i,j));
  return seed;
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
