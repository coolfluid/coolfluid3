// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "math/Consts.hpp"
#include "math/Functions.hpp"

#include "mesh/Octtree.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementFinderOcttree.hpp"


#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;

//////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < ElementFinderOcttree, ElementFinder, LibMesh > ElementFinderOcttree_Builder;

////////////////////////////////////////////////////////////////////////////////

ElementFinderOcttree::ElementFinderOcttree(const std::string &name) : 
  ElementFinder(name),
  m_closest(true)
{
  options().option("dict").attach_trigger( boost::bind( &ElementFinderOcttree::configure_octtree, this ) );

  options().add("find_closest",m_closest)
    .description("If true, an inexact match is allowed, finding the closest element")
    .link_to(&m_closest);

  m_octtree_idx.resize(3);
}

////////////////////////////////////////////////////////////////////////////////

void ElementFinderOcttree::configure_octtree()
{
  Handle<Mesh> mesh = find_parent_component_ptr<Mesh>(*m_dict);
//  std::cout << "configuring octtree for mesh" << mesh->uri() << std::endl;

//  std::cout << OSystem::instance().layer()->back_trace() << std::endl;

  if (is_null(mesh))
    throw SetupError(FromHere(),"Mesh was not found as parent of "+m_dict->uri().string());

  if (Handle<Component> found = mesh->get_child("octtree"))
    m_octtree = Handle<Octtree>(found);
  else
  {
    m_octtree = mesh->create_component<Octtree>("octtree");
    m_octtree->options().set("mesh",mesh);
  }
}

////////////////////////////////////////////////////////////////////////////////

bool ElementFinderOcttree::find_element(const RealVector& target_coord, SpaceElem& element)
{
  cf3_assert(m_octtree);

  if (m_octtree->is_created() == false)
      m_octtree->create_octtree();

  RealVector t_coord(m_octtree->dimension());
  for (Uint d=0; d<target_coord.size(); ++d)
    t_coord[d] = target_coord[d];

  if (m_octtree->find_octtree_cell(t_coord,m_octtree_idx))
  {
    m_elements_pool.clear();
    Uint pool_size = 0;
    Uint rings=0;
    for ( ; pool_size==0 ; ++rings)
    {
      pool_size=m_elements_pool.size();
      m_octtree->gather_elements_around_idx(m_octtree_idx,rings,m_elements_pool);
      boost_foreach(const Entity& pool_elem, boost::make_iterator_range(m_elements_pool.begin()+pool_size,m_elements_pool.end()))
      {
        cf3_assert(is_not_null(pool_elem.comp));
        const RealMatrix elem_coordinates = pool_elem.get_coordinates();
        if (pool_elem.element_type().is_coord_in_element(t_coord,elem_coordinates))
        {
          element = SpaceElem(*const_cast<Space*>(&m_dict->space(*pool_elem.comp)),pool_elem.idx);
          return true;
        }
      }
    }
    // if arrived here, keep searching
    // it means no element has been found. The search is enlarged with one more ring, for possible misses.
    m_octtree->gather_elements_around_idx(m_octtree_idx,rings,m_elements_pool);
    boost_foreach(const Entity& pool_elem, boost::make_iterator_range(m_elements_pool.begin()+pool_size,m_elements_pool.end()))
    {
      cf3_assert(is_not_null(pool_elem.comp));
      const RealMatrix elem_coordinates = pool_elem.get_coordinates();
      if (pool_elem.element_type().is_coord_in_element(t_coord,elem_coordinates))
      {
        element = SpaceElem(*const_cast<Space*>(&m_dict->space(*pool_elem.comp)),pool_elem.idx);
        return true;
      }
    }
  }
  if (m_closest)
  {
//    std::cout << "didnt find element ... will look more in a pool of " << m_elements_pool.size() << std::endl;
    Real distance=math::Consts::real_max();
    int closest_idx=-1;
    RealVector s_elem_centroid = t_coord;
    for (Uint i=0; i<m_elements_pool.size(); ++i)
    {
      int elem_dim=m_elements_pool[i].element_type().dimension();
      m_elements_pool[i].allocate_coordinates(m_coordinates);
      m_elements_pool[i].put_coordinates(m_coordinates);
      m_elements_pool[i].element_type().compute_centroid( m_elements_pool[i].get_coordinates() , s_elem_centroid);

      Real newdistance = math::Functions::get_distance(s_elem_centroid,t_coord);
      if (newdistance < distance)
      {
        distance = newdistance;

        for (Uint n=0; n<m_elements_pool[i].element_type().nb_nodes(); ++n)
        {
          newdistance = math::Functions::get_distance(s_elem_centroid,m_coordinates.row(n));
          if (newdistance>distance)
          {
            closest_idx = i; break;
          }
        }
      }
    }
    if (closest_idx>=0)
    {
//      int elem_dim=m_elements_pool[closest_idx].element_type().dimension();
//      m_elements_pool[closest_idx].allocate_coordinates(m_coordinates);
//      m_elements_pool[closest_idx].put_coordinates(m_coordinates);

//      Real volume = m_elements_pool[closest_idx].element_type().volume( m_coordinates );
//      Real dx3 = std::pow(distance, elem_dim);
//      std::cout << "---> found at distance " << distance << "    vol="<<volume<<"   dx3="<<dx3<< std::endl;
      element = SpaceElem(*const_cast<Space*>(&m_dict->space(*m_elements_pool[closest_idx].comp)),m_elements_pool[closest_idx].idx);
      return true;
    }
  }
//  std::cout << "---> not found" << std::endl;
  // if arrived here, it means no element has been found in the octtree cell. Give up.
  CFdebug << "coord " << t_coord.transpose() << " has not been found in the octtree cell" << CFendl;
  return false;

//  bool found = m_octtree->find_element(target_coord,m_tmp);
//  if (!found)
//    return false;

//  element = SpaceElem(*const_cast<Space*>(&m_dict->space(*m_tmp.comp)),m_tmp.idx);
//  return true;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
