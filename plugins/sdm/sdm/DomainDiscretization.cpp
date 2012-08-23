// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"

#include "common/XML/SignalOptions.hpp"

#include "math/Consts.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Cells.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ShapeFunction.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Solver.hpp"
#include "solver/actions/ForAllCells.hpp"
#include "solver/actions/ForAllFaces.hpp"

#include "sdm/LibSDM.hpp"
#include "sdm/DomainDiscretization.hpp"
#include "sdm/Term.hpp"
#include "sdm/Tags.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < DomainDiscretization, common::Action, LibSDM > DomainDiscretization_Builder;

///////////////////////////////////////////////////////////////////////////////////////

DomainDiscretization::DomainDiscretization ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

  // signals

  regist_signal( "create_term" )
      .connect  ( boost::bind( &DomainDiscretization::signal_create_term, this, _1 ) )
      .signature( boost::bind( &DomainDiscretization::signature_signal_create_term, this, _1))
      .description("creates a discretization term for cells")
      .pretty_name("Create Cell Term");

  m_terms = create_static_component<ActionDirector>("Terms");
}

////////////////////////////////////////////////////////////////////////////////

void DomainDiscretization::execute()
{
  CFdebug << "DomainDiscretization EXECUTE" << CFendl;

  update();

  boost_foreach(const Cells& cells, find_components_recursively<Cells>(mesh()))
  {
    if ( loop_cells(cells.handle<Cells>()) )
    {
      // Element-loop
      const Uint nb_elems = m_cells->size();

      for (Uint elem_idx=0; elem_idx<nb_elems; ++elem_idx)
      {
        if (m_cells->is_ghost(elem_idx)==false)
        {
          compute_element(elem_idx);
        }
      }

    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DomainDiscretization::update()
{
  m_terms_per_cells.clear();
  boost_foreach( Term& term, find_components<Term>(*m_terms))
  {
    boost_foreach(const URI& region_uri, term.options().value< std::vector<URI> >("regions"))
    {
      Handle<Region const> region = mesh().access_component_checked(region_uri)->handle<Region>();
      boost_foreach( const Cells& cells, find_components_recursively<Cells>(*region) )
      {
        m_terms_per_cells[cells.handle<Cells>()].push_back( term.handle<Term>() );
      }
    }
  }
  foreach_container( (const Handle<Cells const>& cells) (std::vector< Handle<Term> >& terms), m_terms_per_cells)
  {
    std::sort(terms.begin(), terms.end());
    terms.erase(std::unique(terms.begin(), terms.end()), terms.end());
  }

  m_residual   = follow_link(solver().field_manager().get_child_checked(sdm::Tags::residual()))->handle<Field>();
  m_wave_speed = follow_link(solver().field_manager().get_child_checked(sdm::Tags::wave_speed()))->handle<Field>();

}

////////////////////////////////////////////////////////////////////////////////

bool DomainDiscretization::loop_cells(const Handle<Cells const>& cells)
{
  if ( m_terms_per_cells.count(cells) == 0)
  {
    return false;
  }

  m_cells = cells;
  m_terms_for_cells = m_terms_per_cells[m_cells];

  boost_foreach( const Handle<Term>& term, m_terms_for_cells )
  {
    term->set_entities(*m_cells);
  }

  m_space = m_residual->space(cells);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DomainDiscretization::compute_element(const Uint elem_idx)
{
  // Reset residual and wave_speed for this element to zero.
  // The term executions add to this.
  for (Uint s=0; s<m_space->shape_function().nb_nodes(); ++s)
  {
    const Uint p = m_space->connectivity()[elem_idx][s];
    for (Uint v=0; v<m_residual->row_size(); ++v)
    {
      m_residual->array()[p][v]=0.;
    }
    m_wave_speed->array()[p][0]=0.;
  }

  // Initialize the terms for this element index
  boost_foreach( const Handle<Term>& term, m_terms_for_cells)
  {
    term->set_element(elem_idx);
  }

  // Execute every term
  boost_foreach( const Handle<Term>& term, m_terms_for_cells)
  {
    term->execute();
  }

  // Unset the element index for the terms
  boost_foreach( const Handle<Term>& term, m_terms_for_cells)
  {
    term->unset_element();
  }
}

////////////////////////////////////////////////////////////////////////////////

Term& DomainDiscretization::create_term( const std::string& type,
                                         const std::string& name,
                                         const std::vector<URI>& regions )
{
  Handle< Term > term = m_terms->create_component<Term>(name, type);

  term->options().set( sdm::Tags::solver(),         solver().handle<Component>());
  term->options().set( sdm::Tags::mesh(),           mesh().handle<Component>());

  if (regions.size() == 0)
    term->options().set("regions", solver().options().value< std::vector<common::URI> >("regions") );
  else
    term->options().set("regions", regions);

  term->options().set( sdm::Tags::physical_model(), physical_model().handle<Component>());

  term->initialize();


  CFinfo << "Created term   " << name << "(" << type << ") for regions " << CFendl;
  boost_foreach(const URI& region_uri, term->options().option("regions").value<std::vector<URI> >())
  {
    cf3_assert(mesh().access_component(region_uri));
    CFinfo << "    - " << mesh().access_component(region_uri)->uri().path() << CFendl;
  }

  return *term;
}

////////////////////////////////////////////////////////////////////////////////

void DomainDiscretization::signal_create_term( SignalArgs& args )
{
  SignalOptions options( args );

  std::string name = options.value<std::string>("name");
  std::string type = options.value<std::string>("type");

  // configure the regions
  // if user did not specify, then use the whole topology (all regions)

  std::vector<URI> regions;
  if( options.check("regions") )
    regions = options.value< std::vector<URI> >("regions");
  else
    regions.push_back(mesh().topology().uri());

  Term& created_component = create_term( type, name, regions );

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", created_component.uri());
}

////////////////////////////////////////////////////////////////////////////////

void DomainDiscretization::signature_signal_create_term( SignalArgs& args )
{
  SignalOptions options( args );

  // name

  options.add("name", std::string() )
      .description("Name for created term");

  // type

  /// @todo loop over the existing CellTerm providers to provide the available list

  options.add("type", std::string("cf3.sdm.Convection"))
      .description("Type for created term");

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted volume regions

  options.add("regions", dummy )
      .description("Regions where to apply the term");
}

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3
