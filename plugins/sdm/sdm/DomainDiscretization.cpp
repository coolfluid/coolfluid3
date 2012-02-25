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

void DomainDiscretization::execute()
{
  Field& residual = *follow_link(solver().field_manager().get_child(sdm::Tags::residual()))->handle<Field>();
  residual = 0.;

  Field& wave_speed = *follow_link(solver().field_manager().get_child(sdm::Tags::wave_speed()))->handle<Field>();
  wave_speed = math::Consts::eps();

//  boost_foreach( Component& term , *m_terms)
//  {
//    term.handle<Term>()->initialize();
//  }

  CFdebug << "DomainDiscretization EXECUTE" << CFendl;
  foreach_container( (const Handle<Region const>& region) (std::vector< Handle<Term> >& terms), m_terms_per_region)
  {
    if (region)
    {
      boost_foreach( const Cells& cells, find_components_recursively<Cells>(*region) )
      {
        boost_foreach( const Handle<Term>& term, terms)
        {
          term->set_entities(cells);
          CFdebug << "DomainDiscretization: executing " << term->name() << " for cells " << cells.uri() << CFendl;
          for (Uint elem_idx=0; elem_idx<cells.size(); ++elem_idx)
          {
            if (cells.is_ghost(elem_idx)==false)
            {
              term->set_element(elem_idx);
              term->execute();
              term->unset_element();
            }
          }
        }
      }
    }
  }
}

Term& DomainDiscretization::create_term( const std::string& type,
                                         const std::string& name,
                                         const std::vector<URI>& regions )
{
  Handle< Term > term = m_terms->create_component<Term>(name, type);

  term->options().configure_option( sdm::Tags::solver(),         solver().handle<Component>());
  term->options().configure_option( sdm::Tags::mesh(),           mesh().handle<Component>());

  if (regions.size() == 0)
    term->options().configure_option("regions", solver().options().option("regions").value< std::vector<common::URI> >() );
  else
    term->options().configure_option("regions", regions);

  term->options().configure_option( sdm::Tags::physical_model(), physical_model().handle<Component>());

  term->initialize();

  const std::string option_name("regions");
  boost_foreach(const URI& region_uri, term->options().option(option_name).value<std::vector<URI> >())
  {
    cf3_assert(mesh().access_component(region_uri));
    m_terms_per_region[mesh().access_component(region_uri)->handle<Region>()].push_back(term->handle<Term>());
  }

  CFinfo << "Created term   " << name << "(" << type << ") for regions " << CFendl;
  boost_foreach(const URI& region_uri, term->options().option(option_name).value<std::vector<URI> >())
  {
    cf3_assert(mesh().access_component(region_uri));
    CFinfo << "    - " << mesh().access_component(region_uri)->uri().path() << CFendl;
  }

  return *term;
}

void DomainDiscretization::signal_create_term( SignalArgs& args )
{
  SignalOptions options( args );

  std::string name = options.value<std::string>("name");
  std::string type = options.value<std::string>("type");

  // configure the regions
  // if user did not specify, then use the whole topology (all regions)

  std::vector<URI> regions;
  if( options.check("regions") )
    regions = options.array<URI>("regions");
  else
    regions.push_back(mesh().topology().uri());

  Term& created_component = create_term( type, name, regions );

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("created_component", created_component.uri());
}


void DomainDiscretization::signature_signal_create_term( SignalArgs& args )
{
  SignalOptions options( args );

  // name

  options.add_option("name", std::string() )
      .description("Name for created term");

  // type

  /// @todo loop over the existing CellTerm providers to provide the available list

  options.add_option("type", std::string("cf3.sdm.Convection"))
      .description("Type for created term");

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted volume regions

  options.add_option("regions", dummy )
      .description("Regions where to apply the term");
}

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3
