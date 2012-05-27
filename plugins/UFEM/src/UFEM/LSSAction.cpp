// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include <common/List.hpp>
#include <common/PropertyList.hpp>

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/Tags.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

#include "physics/PhysModel.hpp"

#include "LSSAction.hpp"
#include "SparsityBuilder.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace math;
using namespace mesh;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

common::ComponentBuilder < LSSAction, common::ActionDirector, LibUFEM > LSSAction_Builder;

struct LSSAction::Implementation
{
  Implementation(Component& comp) :
   m_component(comp),
   system_matrix(m_component.options().add_option("lss", Handle<LSS::System>()).pretty_name("LSS")),
   system_rhs(m_component.options().option("lss")),
   dirichlet(m_component.options().option("lss")),
   solution(m_component.options().option("lss")),
   m_updating(false)
  {
  }

  Component& m_component;
  SystemMatrix system_matrix;
  SystemRHS system_rhs;
  DirichletBC dirichlet;
  SolutionVector solution;

  Handle<LSS::System> m_lss;

  bool m_updating;
};

LSSAction::LSSAction(const std::string& name) :
  solver::ActionDirector(name),
  m_implementation( new Implementation(*this) ),
  system_matrix(m_implementation->system_matrix),
  system_rhs(m_implementation->system_rhs),
  dirichlet(m_implementation->dirichlet),
  solution(m_implementation->solution)
{
  properties().add_property("solution_tag", std::string(UFEM::Tags::solution()));

  regist_signal( "create_lss" )
    .connect( boost::bind( &LSSAction::signal_create_lss, this, _1 ) )
    .description("Create the Linear System Solver")
    .pretty_name("Create LSS")
    .signature( boost::bind( &LSSAction::signature_create_lss, this, _1 ) );

  options().add_option("dictionary", m_dictionary)
    .pretty_name("Dictionary")
    .description("The dictionary to use for field lookups")
    .link_to(&m_dictionary);
}

LSSAction::~LSSAction()
{
}

void LSSAction::execute()
{
  if(is_null(m_implementation->m_lss))
    throw SetupError(FromHere(), "Error executing " + uri().string() + ": Invalid LSS");

  solver::ActionDirector::execute();
}

LSS::System& LSSAction::create_lss(const std::string& matrix_builder)
{
  Handle<LSS::System> lss = create_component<LSS::System>("LSS");
  lss->options().set("matrix_builder", matrix_builder);

  configure_option_recursively("lss", lss);

  on_regions_set();

  return *lss;
}

void LSSAction::signature_create_lss(SignalArgs& node)
{
  SignalOptions options(node);
  options.add_option("matrix_builder", "cf3.math.LSS.TrilinosFEVbrMatrix")
    .pretty_name("Matrix Builder")
    .description("Name for the matrix builder to use when constructing the LSS");
}

void LSSAction::signal_create_lss(SignalArgs& node)
{
  SignalOptions options(node);
  LSS::System& lss = create_lss(options.option("matrix_builder").value<std::string>());

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("created_component", lss.uri());
}


void LSSAction::on_regions_set()
{
  if(m_implementation->m_updating) // avoid recursion
    return;
  
  m_implementation->m_lss = options().option("lss").value< Handle<LSS::System> >();
  if(is_null(m_implementation->m_lss))
    return;

  if(is_null(m_dictionary))
    return;

  m_implementation->m_updating = true;

  // Create the LSS if the mesh is set
  if(!m_loop_regions.empty() && !m_implementation->m_lss->is_created())
  {
    VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(physical_model().variable_manager(), solution_tag());

    Handle< List<Uint> > gids = m_implementation->m_lss->create_component< List<Uint> >("GIDs");
    Handle< List<Uint> > ranks = m_implementation->m_lss->create_component< List<Uint> >("Ranks");
    Handle< List<Uint> > used_node_map = m_implementation->m_lss->create_component< List<Uint> >("used_node_map");

    std::vector<Uint> node_connectivity, starting_indices;
    boost::shared_ptr< List<Uint> > used_nodes = build_sparsity(m_loop_regions, *m_dictionary, node_connectivity, starting_indices, *gids, *ranks, *used_node_map);
    add_component(used_nodes);

    // This comm pattern is valid only over the used nodes for the supplied regions
    PE::CommPattern& comm_pattern = *create_component<PE::CommPattern>("CommPattern");
    comm_pattern.insert("gid",gids->array(),false);
    comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),ranks->array());

    CFdebug << "Creating LSS for " << starting_indices.size()-1 << " blocks with descriptor " << solution_tag() << ": " << descriptor.description() << CFendl;
    m_implementation->m_lss->create(comm_pattern, descriptor.size(), node_connectivity, starting_indices);
    CFdebug << "Finished creating LSS" << CFendl;
    configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
    configure_option_recursively("lss", m_implementation->m_lss);
  }

  m_implementation->m_updating = false;
}

void LSSAction::trigger_dictionary()
{
  if(is_not_null(m_dictionary))
  {
    boost_foreach(Component& child, *this)
    {
      child.configure_option_recursively("dictionary", m_dictionary);
    }
    on_regions_set();
  }
}

std::string LSSAction::solution_tag()
{
  return properties().value<std::string>("solution_tag");
}

void LSSAction::set_solution_tag(const std::string& tag)
{
  properties().configure_property("solution_tag", tag);
}



} // UFEM
} // cf3
