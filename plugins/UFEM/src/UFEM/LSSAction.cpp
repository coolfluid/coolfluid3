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
   system_matrix(m_component.options().add("lss", Handle<LSS::System>()).pretty_name("LSS")),
   system_rhs(m_component.options().option("lss")),
   dirichlet(m_component.options().option("lss")),
   solution(m_component.options().option("lss")),
   m_updating(false)
  {
    m_component.options().option("lss").attach_trigger(boost::bind(&Implementation::trigger_lss, this));
  }

  void trigger_lss()
  {
    m_lss = m_component.options().option("lss").value< Handle<LSS::System> >();
    CFdebug << "lss for " << m_component.uri().path() << " set to " << m_component.options().option("lss").value_str() << CFendl;
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
  properties().add("solution_tag", std::string(UFEM::Tags::solution()));

  regist_signal( "create_lss" )
    .connect( boost::bind( &LSSAction::signal_create_lss, this, _1 ) )
    .description("Create the Linear System Solver")
    .pretty_name("Create LSS");

  options().add("dictionary", m_dictionary)
    .pretty_name("Dictionary")
    .description("The dictionary to use for field lookups")
    .link_to(&m_dictionary);

  options().add("initial_conditions", m_initial_conditions)
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to")
    .link_to(&m_initial_conditions)
    .attach_trigger(boost::bind(&LSSAction::trigger_initial_conditions, this));

  options().add("blocked_system", false)
    .pretty_name("Blocked System")
    .description("Store the linear system internally as a set of blocks grouped per variable, rather than keeping the variables per node");

  options().add("matrix_builder", "cf3.math.LSS.TrilinosFEVbrMatrix")
    .pretty_name("Matrix Builder")
    .description("Builder to use when creating the LSS")
    .mark_basic();

  options().add("solution_strategy", "cf3.math.LSS.TrilinosStratimikosStrategy")
    .pretty_name("Solution Strategy")
    .description("Builder to use when creating the initial LSS solution strategy")
    .mark_basic();
}

LSSAction::~LSSAction()
{
}

void LSSAction::execute()
{
  if(is_null(m_implementation->m_lss))
  {
    throw SetupError(FromHere(), "Error executing " + uri().string() + ": LSS is not created");
  }

  CFdebug << "Running with LSS " << options().option("lss").value_str() << CFendl;

  solver::ActionDirector::execute();
}

LSS::System& LSSAction::create_lss()
{
  if(is_not_null(get_child("LSS")))
    remove_component("LSS");
  Handle<LSS::System> lss = create_component<LSS::System>("LSS");
  lss->mark_basic();
  lss->options().set("matrix_builder", options().option("matrix_builder").value());
  lss->options().set("solution_strategy", options().option("solution_strategy").value());
  
  configure_option_recursively("lss", lss);

  cf3_assert(is_not_null(options().value< Handle<LSS::System> >("lss")));

  on_regions_set();

  return *lss;
}

void LSSAction::signal_create_lss(SignalArgs& node)
{
  LSS::System& lss = create_lss();

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", lss.uri());
}


void LSSAction::on_regions_set()
{
  if(m_implementation->m_updating) // avoid recursion
  {
    CFdebug << "Skipping on_regions_set to avoid recursion" << CFendl;
    return;
  }

  m_implementation->m_lss = options().value< Handle<LSS::System> >("lss");
  if(is_null(m_implementation->m_lss))
  {
    create_lss();
  }

  if(is_null(m_dictionary))
  {
    CFdebug << "Skipping on_regions_set because dictionary is null" << CFendl;
    return;
  }

  m_implementation->m_updating = true;

  // Create the LSS if the mesh is set
  if(!m_loop_regions.empty() && !m_implementation->m_lss->is_created())
  {
    VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(physical_model().variable_manager(), solution_tag());

    Handle< List<Uint> > gids = m_implementation->m_lss->create_component< List<Uint> >("GIDs");
    Handle< List<Uint> > ranks = m_implementation->m_lss->create_component< List<Uint> >("Ranks");
    Handle< List<int> > used_node_map = m_implementation->m_lss->create_component< List<int> >("used_node_map");

    std::vector<Uint> node_connectivity, starting_indices;
    boost::shared_ptr< List<Uint> > used_nodes = build_sparsity(m_loop_regions, *m_dictionary, node_connectivity, starting_indices, *gids, *ranks, *used_node_map);
    if(is_not_null(get_child(used_nodes->name())))
      remove_component(used_nodes->name());
    add_component(used_nodes);

    // This comm pattern is valid only over the used nodes for the supplied regions
    if(is_not_null(get_child("CommPattern")))
      remove_component("CommPattern");
    PE::CommPattern& comm_pattern = *create_component<PE::CommPattern>("CommPattern");
    comm_pattern.insert("gid",gids->array(),false);
    comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),ranks->array());

    if(is_not_null(m_dictionary->get_child("node_gids")))
    {
      Field& node_gids = *(Handle<Field>(m_dictionary->get_child("node_gids")));
      const Uint nb_nodes = node_gids.size();

      for(Uint i = 0; i != nb_nodes; ++i)
      {
        node_gids[i][0] = -1.;
      }
      const Uint nb_used = used_nodes->size();
      for(Uint i = 0; i != nb_used; ++i)
      {
        node_gids[used_nodes->array()[i]][0] = gids->array()[i];
      }
    }

    // Build node periodicity based on the used nodes, if needed
    std::vector<Uint> periodic_links_nodes_vec;
    std::vector<bool> periodic_links_active_vec;

    Handle< List<Uint> > periodic_links_nodes_h(m_dictionary->get_child("periodic_links_nodes"));
    Handle< List<bool> > periodic_links_active_h(m_dictionary->get_child("periodic_links_active"));
    if(is_not_null(periodic_links_nodes_h))
    {
      const List<Uint>& periodic_links_nodes = *periodic_links_nodes_h;
      const List<bool>& periodic_links_active = *periodic_links_active_h;
      const List<Uint>& used_nodes_list = *used_nodes;
      const Uint nb_used_nodes = used_nodes_list.size();
      periodic_links_active_vec.resize(nb_used_nodes, false);
      periodic_links_nodes_vec.resize(nb_used_nodes);
      for(Uint i = 0; i != nb_used_nodes; ++i)
      {
        if(periodic_links_active[used_nodes_list[i]])
        {
          periodic_links_active_vec[i] = true;
          periodic_links_nodes_vec[i] = (*used_node_map)[periodic_links_nodes[used_nodes_list[i]]];
        }
      }
    }

    do_create_lss(comm_pattern, descriptor, node_connectivity, starting_indices, periodic_links_nodes_vec, periodic_links_active_vec);

    CFdebug << "Finished creating LSS" << CFendl;
    configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
    configure_option_recursively("lss", m_implementation->m_lss);
  }
  else
  {
    if(m_loop_regions.empty())
      CFdebug << "Skipping on_regions_set because region list is empty" << CFendl;
    else
      CFdebug << "Skipping on_regions_set because LSS is already created" << CFendl;
  }

  // Update the regions of any owned initial conditions
  BOOST_FOREACH(const Handle<Component>& ic, m_created_initial_conditions)
  {
    if(is_not_null(ic))
      ic->options().set(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
  }

  cf3_assert(is_not_null(m_implementation->m_lss));

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

void LSSAction::trigger_initial_conditions()
{
  if(is_null(m_initial_conditions))
  {
    CFwarn << "Initial conditions for " << uri().path() << " were reset to NULL" << CFendl;
    return;
  }

  CFdebug << "Using initial conditions " << m_initial_conditions->uri().path() << " for LSSAction " << uri().path() << CFendl;

  m_created_initial_conditions.clear();

  std::set< Handle<Component> > existing_conditions;
  BOOST_FOREACH(common::Action& ic, find_components<common::Action>(*m_initial_conditions))
  {
    existing_conditions.insert(ic.handle());
  }

  // Give the concrete solver a chance to add its initial conditions
  on_initial_conditions_set(*m_initial_conditions);

  // New initial conditions are the ones we need to track
  BOOST_FOREACH(common::Action& ic, find_components<common::Action>(*m_initial_conditions))
  {
    if(!existing_conditions.count(ic.handle()))
    {
      m_created_initial_conditions.push_back(ic.handle());
      CFdebug << "  added initial condition " << ic.name() << CFendl;
    }
  }
}


std::string LSSAction::solution_tag()
{
  return properties().value<std::string>("solution_tag");
}

void LSSAction::set_solution_tag(const std::string& tag)
{
  properties().set("solution_tag", tag);
}

void LSSAction::on_initial_conditions_set(InitialConditions& initial_conditions)
{
}

void LSSAction::do_create_lss(PE::CommPattern &cp, const VariablesDescriptor &vars, std::vector<Uint> &node_connectivity, std::vector<Uint> &starting_indices, const std::vector<Uint> &periodic_links_nodes, const std::vector<bool> &periodic_links_active)
{
  const bool blocked_system = options().option("blocked_system").value<bool>();
  if(blocked_system)
    CFdebug << "Creating blocked LSS for ";
  else
    CFdebug << "Creating per-node LSS for ";
  CFdebug <<  starting_indices.size()-1 << " blocks with descriptor " << solution_tag() << ": " << vars.description() << CFendl;

  if(blocked_system)
    m_implementation->m_lss->create_blocked(cp, vars, node_connectivity, starting_indices, periodic_links_nodes, periodic_links_active);
  else
    m_implementation->m_lss->create(cp, vars.size(), node_connectivity, starting_indices, periodic_links_nodes, periodic_links_active);
}

} // UFEM
} // cf3
