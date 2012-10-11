// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"

#include "common/XML/SignalOptions.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/ConnectivityData.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Tags.hpp"

#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace common::XML;
using namespace math;
using namespace mesh;
using namespace solver;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < AdjacentCellToFace, common::Action, LibUFEM > AdjacentCellToFace_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

AdjacentCellToFace::AdjacentCellToFace(const std::string& name) : solver::Action(name)
{
  m_node_connectivity = create_static_component<CNodeConnectivity>("NodeConnectivity");

  options().add("field_tag", "")
    .pretty_name("Field Tag")
    .description("Tag for the field in which the initial conditions will be set");
}

AdjacentCellToFace::~AdjacentCellToFace()
{
}

void AdjacentCellToFace::on_regions_set()
{
  if(m_loop_regions.empty())
    return;

  mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(*m_loop_regions.front());
  m_node_connectivity->initialize(find_components_recursively_with_filter<Elements>(mesh, IsElementsVolume()));

  std::set<std::string> adjacent_regions;

  BOOST_FOREACH(Handle<Region> region, m_loop_regions)
  {
    BOOST_FOREACH(Elements& elements, find_components_recursively_with_filter<Elements>(*region, IsElementsSurface()))
    {
      Handle<CFaceConnectivity> face_conn(find_component_ptr_with_tag(elements, "face_to_cell_connectivity"));
      if(is_null(face_conn))
      {
        face_conn = elements.create_component<CFaceConnectivity>("FaceToCellConnectivity");
        face_conn->add_tag("face_to_cell_connectivity");
        face_conn->initialize(*m_node_connectivity);
      }

      const Uint nb_elems = elements.size();
      for(Uint i = 0; i != nb_elems; ++i)
      {
        cf3_assert(face_conn->has_adjacent_element(i, 0));
        const CFaceConnectivity::ElementReferenceT adj_elem = face_conn->adjacent_element(i, 0);
        adjacent_regions.insert(adj_elem.first->parent()->name());
      }
    }
  }

  CFdebug << "AdjacentCellToFace at " << uri().path() << " uses adjacent regions ";
  BOOST_FOREACH(const std::string& region, adjacent_regions)
  {
    CFdebug << region << " ";
  }
  CFdebug << CFendl;
}


void AdjacentCellToFace::execute()
{
  const std::string field_tag = options()["field_tag"].value<std::string>();
  if(field_tag.empty())
    throw common::SetupError(FromHere(), "field_tag option is not set for " + uri().path());

  BOOST_FOREACH(const Handle<Region>& region, m_loop_regions)
  {
    mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(*region);
    mesh::Field& field = common::find_component_recursively_with_tag<mesh::Field>(mesh, field_tag);
    BOOST_FOREACH(Elements& elements, find_components_recursively_with_filter<Elements>(*region, IsElementsSurface()))
    {
      CFaceConnectivity& face_conn = find_component_with_tag<CFaceConnectivity>(elements, "face_to_cell_connectivity");
      const Uint own_field_elements_begin = field.dict().space(elements).connectivity()[0][0];
      const Uint nb_elems = elements.size();
      for(Uint i = 0; i != nb_elems; ++i)
      {
        cf3_assert(face_conn.has_adjacent_element(i, 0));
        const CFaceConnectivity::ElementReferenceT adj_elem = face_conn.adjacent_element(i, 0);
        const Uint other_field_elements_begin = field.dict().space(*adj_elem.first).connectivity()[0][0];
        field.set_row(own_field_elements_begin + i, field[other_field_elements_begin + adj_elem.second]);
      }
    }
  }
}



} // UFEM
} // cf3
