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

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"

#include "physics/PhysModel.hpp"

#include "solver/CSolver.hpp"
#include "solver/actions/CForAllCells.hpp"
#include "solver/actions/CForAllFaces.hpp"

#include "SFDM/DomainDiscretization.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/Tags.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace SFDM {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < DomainDiscretization, common::Action, LibSFDM > DomainDiscretization_Builder;

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
//  CFinfo << "[SFDM] applying domain discretization" << CFendl;

  // compute first the cell terms, since they may store something for faces to use

  //CFinfo << "  terms()" << CFendl;
  m_terms->execute();
}

Term& DomainDiscretization::create_term( const std::string& type,
                                         const std::string& name,
                                         const std::vector<URI>& regions )
{
  CFinfo << "Creating cell term   " << name << "(" << type << ")" << CFendl;
  Handle< Term > term = m_terms->create_component<Term>(name, type);

  if (regions.size() == 0)
    term->options().configure_option("regions", std::vector<URI>(1,mesh().topology().uri()));
  else
    term->options().configure_option("regions", regions);

  term->options().configure_option( SFDM::Tags::mesh(),           mesh().handle<Component>());
  term->options().configure_option( SFDM::Tags::solver(),         solver().handle<Component>());
  term->options().configure_option( SFDM::Tags::physical_model(), physical_model().handle<Component>());

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

  create_term( type, name, regions );
}


void DomainDiscretization::signature_signal_create_term( SignalArgs& args )
{
  SignalOptions options( args );

  // name

  options.add_option("name", std::string() )
      .description("Name for created term");

  // type

  /// @todo loop over the existing CellTerm providers to provide the available list

  options.add_option("type", std::string("cf3.SFDM.Convection"))
      .description("Type for created term");

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted volume regions

  options.add_option("regions", dummy )
      .description("Regions where to apply the term");
}

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // cf3
