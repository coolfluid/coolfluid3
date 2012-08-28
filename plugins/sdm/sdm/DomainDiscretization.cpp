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
#include "sdm/ElementCaching.hpp"

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
  cf3::common::Action(name)
{
  mark_basic();

  options().add(sdm::Tags::solution(),m_solution).link_to(&m_solution);
  options().add(sdm::Tags::residual(),m_residual).link_to(&m_residual);
  options().add(sdm::Tags::wave_speed(),m_wave_speed).link_to(&m_wave_speed);

  // signals
  regist_signal( "create_term" )
      .connect  ( boost::bind( &DomainDiscretization::signal_create_term, this, _1 ) )
      .signature( boost::bind( &DomainDiscretization::signature_signal_create_term, this, _1))
      .description("creates a discretization term for cells")
      .pretty_name("Create Cell Term");

  m_shared_caches = create_component<SharedCaches>(Tags::shared_caches());

}

////////////////////////////////////////////////////////////////////////////////

void DomainDiscretization::execute()
{
  CFdebug << "DomainDiscretization EXECUTE" << CFendl;

  update();

  boost_foreach(const Handle<Entities>& cells, m_solution->entities_range())
  {
    if ( loop_cells(cells) )
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
  // Configure the solution, residual, wave speed
  if ( is_null(m_solution) )   throw SetupError(FromHere(), "solution not configured");
  if ( is_null(m_residual) )   throw SetupError(FromHere(), "residual not configured");
  if ( is_null(m_wave_speed) ) throw SetupError(FromHere(), "wave_speed not configured");
  m_terms_vector.clear();
  boost_foreach( Term& term, find_components<Term>(*this))
  {
    m_terms_vector.push_back(term.handle<Term>());
    term.options().set("solution",m_solution);
    term.options().set("residual",m_residual);
    term.options().set("wave_speed",m_wave_speed);

    term.initialize();
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DomainDiscretization::loop_cells(const Handle<Entities const>& cells)
{
  if ( is_null(cells->handle<Cells>()) )
    return false;

  m_cells = cells->handle<Cells>();
  boost_foreach( Term& term, find_components<Term>(*this))
  {
    term.set_entities(*m_cells);
  }

  m_space = m_residual->space(m_cells);
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
  boost_foreach( const Handle<Term>& term, m_terms_vector)
  {
    term->set_element(elem_idx);
  }

  // Execute every term
  boost_foreach( const Handle<Term>& term, m_terms_vector)
  {
    term->execute();
  }

  // Unset the element index for the terms
  boost_foreach( const Handle<Term>& term, m_terms_vector)
  {
    term->unset_element();
  }
}

////////////////////////////////////////////////////////////////////////////////

Term& DomainDiscretization::create_term(const std::string& name,const std::string& type)
{
  Handle< Term > term = create_component<Term>(name, type);

  term->options().set("shared_caches",m_shared_caches);

  CFinfo << "Created term   " << name << "(" << type << ")" << CFendl;

  return *term;
}

////////////////////////////////////////////////////////////////////////////////

void DomainDiscretization::signal_create_term( SignalArgs& args )
{
  SignalOptions options( args );

  std::string name = options.value<std::string>("name");
  std::string type = options.value<std::string>("type");

  Term& created_component = create_term( name, type);

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
}

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3
