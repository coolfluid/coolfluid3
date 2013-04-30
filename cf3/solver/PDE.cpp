// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/Option.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"
#include "common/FindComponents.hpp"
#include "common/ActionDirector.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/PDE.hpp"
#include "solver/Time.hpp"
#include "solver/Term.hpp"
#include "solver/BC.hpp"
#include "solver/ComputeRHS.hpp"

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < PDE, common::Component, LibSolver > PDE_Builder;

////////////////////////////////////////////////////////////////////////////////

PDE::PDE ( const std::string& name  ) :
  common::Component ( name )
{
  // properties

  properties()["brief"] = std::string("Spectral Finite Difference PDE");
  properties()["description"] = std::string("Long description not available");


  options().add("fields", m_fields )
      .mark_basic()
      .link_to(&m_fields)
      .attach_trigger( boost::bind( &PDE::create_fields, this) );
  options().add("solution", m_solution ).mark_basic().link_to(&m_solution);
  options().add("rhs", m_rhs ).mark_basic().link_to(&m_rhs);
  options().add("wave_speed", m_wave_speed ).mark_basic().link_to(&m_wave_speed);

  m_nb_eqs = 0u;

  m_rhs_computer = create_static_component<solver::ComputeRHS>("rhs_computer");
  m_rhs_computer->mark_basic();

  m_bc = create_static_component<common::ActionDirector>("bc");
  m_bc->mark_basic();

  regist_signal ( "add_term" )
      .description( "Add term" )
      .pretty_name("Add term" )
      .connect   ( boost::bind ( &PDE::signal_add_term,    this, _1 ) )
      .signature ( boost::bind ( &PDE::signature_add_term, this, _1 ) );

  regist_signal ( "add_bc" )
      .description( "Add boundary condition" )
      .pretty_name("Add BC" )
      .connect   ( boost::bind ( &PDE::signal_add_bc,    this, _1 ) )
      .signature ( boost::bind ( &PDE::signature_add_bc, this, _1 ) );

}

////////////////////////////////////////////////////////////////////////////////

PDE::~PDE()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string PDE::solution_variables() const
{
  return "solution["+to_str( nb_eqs() )+"]";
}

////////////////////////////////////////////////////////////////////////////////

void PDE::create_fields()
{
  if (m_nb_eqs == 0)
    throw InvalidStructure(FromHere(), "PDE "+derived_type_name()+" does not have any equations defined");
  if (is_null(m_fields))
    throw SetupError(FromHere(), "Dictionary in PDE "+uri().string()+" is not setup correctly");
  if ( is_null(m_solution) || ( &m_solution->dict() != m_fields.get() ) )
  {
    if ( Handle<Component> found = m_fields->get_child("solution") )
    {
      m_solution = found->handle<Field>();
    }
    else
    {
      m_solution = m_fields->create_field("solution",solution_variables()).handle<Field>();
      m_solution->parallelize();
    }
    options().set("solution",m_solution);
  }
  if ( is_null(m_rhs) || ( &m_rhs->dict() != m_fields.get() ) )
  {
    if ( Handle<Component> found = m_fields->get_child("rhs") )
    {
      m_rhs = found->handle<Field>();
    }
    else
    {
      m_rhs = m_fields->create_field("rhs",m_nb_eqs).handle<Field>();
      m_rhs->parallelize();
    }
    options().set("rhs",m_rhs);
  }
  if ( is_null(m_wave_speed) || ( &m_wave_speed->dict() != m_fields.get() ) )
  {
    if ( Handle<Component> found = m_fields->get_child("wave_speed") )
    {
      m_wave_speed = found->handle<Field>();
    }
    else
    {
      m_wave_speed = m_fields->create_field("wave_speed",1u).handle<Field>();
      m_wave_speed->parallelize();
    }
    options().set("wave_speed",m_wave_speed);
  }
  m_rhs_computer->options().set("rhs",m_rhs);
  m_rhs_computer->options().set("wave_speed",m_wave_speed);
}

////////////////////////////////////////////////////////////////////////////////

void PDE::configure(const Handle<Component>& component)
{
  cf3_assert( is_not_null(component) );
  foreach_container( (const std::string& opt_name) (boost::shared_ptr<Option>& opt) , options() )
  {
    if ( component->options().check(opt_name) )
    {
      opt->link_option(component->options().option_ptr(opt_name));
      component->options().set(opt_name,opt->value());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

Handle<solver::Time> PDE::add_time()
{
  m_time = create_component<solver::Time>("time");
  m_time->mark_basic();
  options().add("time",m_time);
  boost_foreach( solver::Term& term, find_components<solver::Term>(*this) )
  {
    term.options().set("time",m_time);
  }
  boost_foreach( solver::BC& bc, find_components<solver::BC>(*this) )
  {
    bc.options().set("time",m_time);
  }

  return m_time;
}

////////////////////////////////////////////////////////////////////////////////

Handle<solver::Term> PDE::add_term(const std::string& term_name, const std::string& term_type, const std::string& term_computer_type)
{
  // - Create term
  Handle<solver::Term> term = create_component<solver::Term>(term_name,term_type);
  // - Link all the PDE's configuration options to the term
  configure(term->handle());

  std::string tct = ( ! term_computer_type.empty() ? term_computer_type : term_type+"Computer" );
  CFinfo << "term_computer = " << tct << CFendl;
  Handle<Component> term_computer = m_rhs_computer->create_component(term_name+"_computer",tct);
  term_computer->mark_basic();
  term_computer->options().set("term",term->handle());

  // - Return the created term
  return term;
}

////////////////////////////////////////////////////////////////////////////////

Handle<solver::BC> PDE::add_bc( const std::string& bc_name,
                                const std::string& bc_type,
                                const std::vector< Handle<Component> >& regions )
{
  Handle<solver::BC> bc = m_bc->create_component<solver::BC>(bc_name,bc_type);
  bc->options()["regions"] = regions;
  cf3_always_assert(regions.size());
  cf3_always_assert(bc->options().value< std::vector< Handle<Component> > >("regions").size());
  configure(bc->handle());
  return bc;
}

////////////////////////////////////////////////////////////////////////////////

void PDE::signal_add_term( common::SignalArgs& args )
{
  common::XML::SignalOptions opts(args);

  Handle<solver::Term> term = add_term( opts.value<std::string>("name"),
                                        opts.value<std::string>("type"),
                                        opts.value<std::string>("computer"));

  common::XML::SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component",term->uri());

}

void PDE::signature_add_term( common::SignalArgs& args )
{
  common::XML::SignalOptions opts(args);
  opts.add("name",std::string());
  opts.add("type",std::string());
  opts.add("computer",std::string());
}

////////////////////////////////////////////////////////////////////////////////

void PDE::signal_add_bc( common::SignalArgs& args )
{
  common::XML::SignalOptions opts(args);

  Handle<solver::BC> bc = add_bc( opts.value<std::string>("name"),
                                  opts.value<std::string>("type"),
                                  opts.value< std::vector< Handle<Component> > >("regions"));

  common::XML::SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component",bc->uri());

}

void PDE::signature_add_bc( common::SignalArgs& args )
{
  common::XML::SignalOptions opts(args);
  opts.add("name",std::string());
  opts.add("type",std::string());
  opts.add("regions",std::vector< Handle<Component> >());
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
