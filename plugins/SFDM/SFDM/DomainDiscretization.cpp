// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/Actions/CForAllCells.hpp"
#include "Solver/Actions/CForAllFaces.hpp"

#include "SFDM/DomainDiscretization.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/Tags.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace SFDM {


///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < DomainDiscretization, CAction, LibSFDM > DomainDiscretization_Builder;

///////////////////////////////////////////////////////////////////////////////////////

DomainDiscretization::DomainDiscretization ( const std::string& name ) :
  CF::Solver::ActionDirector(name)
{
  mark_basic();

  // signals

  regist_signal( "create_term" )
      ->connect  ( boost::bind( &DomainDiscretization::signal_create_term, this, _1 ) )
      ->signature( boost::bind( &DomainDiscretization::signature_signal_create_term, this, _1))
      ->description("creates a discretization term for cells")
      ->pretty_name("Create Cell Term");

  m_terms = create_static_component_ptr<CActionDirector>("Terms");
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
  Term::Ptr term = build_component_abstract_type<Term>(type,name);
  m_terms->append(term);

  if (regions.size() == 0)
    term->configure_option("regions", std::vector<URI>(1,mesh().topology().uri()));
  else
    term->configure_option("regions", regions);

  term->configure_option( SFDM::Tags::mesh(),           mesh().uri());
  term->configure_option( SFDM::Tags::solver(),         solver().uri());
  term->configure_option( SFDM::Tags::physical_model(), physical_model().uri());

  return *term;
}

void DomainDiscretization::signal_create_term( SignalArgs& args )
{
  SignalOptions options( args );

  std::string name = options.value<std::string>("Name");
  std::string type = options.value<std::string>("Type");

  // configure the regions
  // if user did not specify, then use the whole topology (all regions)

  std::vector<URI> regions;
  if( options.check("Regions") )
    regions = options.array<URI>("Regions");
  else
    regions.push_back(mesh().topology().uri());

  create_term( type, name, regions );
}


void DomainDiscretization::signature_signal_create_term( SignalArgs& args )
{
  SignalOptions options( args );

  // name

  options.add_option< OptionT<std::string> >("Name", std::string() )
      ->description("Name for created term");

  // type

  /// @todo loop over the existing CellTerm providers to provide the available list

  //  std::vector< std::string > restricted;
  //  restricted.push_back( std::string("CF.SFDM.BcDirichlet") );
  //  XmlNode type_node = options.add_option< OptionT<std::string> >("Type", std::string("CF.SFDM.BcDirichlet"), "Type for created boundary");
  //  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted volume regions

  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->description("Regions where to apply the domain term");
}

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // CF
