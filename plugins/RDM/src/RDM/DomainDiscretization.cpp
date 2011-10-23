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
#include "RDM/Tags.hpp"

#include "RDM/CellTerm.hpp"
#include "RDM/FaceTerm.hpp"

#include "DomainDiscretization.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;

namespace cf3 {
namespace RDM {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < DomainDiscretization, common::Action, LibRDM > DomainDiscretization_Builder;

///////////////////////////////////////////////////////////////////////////////////////

DomainDiscretization::DomainDiscretization ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

  // signals

  regist_signal( "create_cell_term" )
      ->connect  ( boost::bind( &DomainDiscretization::signal_create_cell_term, this, _1 ) )
      ->signature( boost::bind( &DomainDiscretization::signature_signal_create_cell_term, this, _1))
      ->description("creates a discretization term for cells")
      ->pretty_name("Create Cell Term");

  regist_signal( "create_face_term" )
      ->connect  ( boost::bind( &DomainDiscretization::signal_create_face_term, this, _1 ) )
      ->signature( boost::bind( &DomainDiscretization::signature_signal_create_face_term, this, _1))
      ->description("creates a discretization term for faces")
      ->pretty_name("Create Cell Term");


  m_face_terms = create_static_component_ptr<ActionDirector>("FaceTerms");
  m_cell_terms = create_static_component_ptr<ActionDirector>("CellTerms");
}


void DomainDiscretization::execute()
{
//  CFinfo << "[RDM] applying domain discretization" << CFendl;

  // compute first the cell terms, since they may store something for faces to use

  m_cell_terms->execute();

  // compute the face terms

  m_face_terms->execute();

}

RDM::CellTerm& DomainDiscretization::create_cell_term( const std::string& type,
                                                       const std::string& name,
                                                       const std::vector<URI>& regions )
{
  RDM::CellTerm::Ptr term = build_component_abstract_type<RDM::CellTerm>(type,name);

  m_cell_terms->append(term);

  term->configure_option("regions" , regions);

  term->configure_option( RDM::Tags::mesh(), m_mesh.lock()->uri());
  term->configure_option( RDM::Tags::solver() , m_solver.lock()->uri());
  term->configure_option( RDM::Tags::physical_model() , m_physical_model.lock()->uri());

  return *term;
}

RDM::FaceTerm& DomainDiscretization::create_face_term( const std::string& type,
                                                       const std::string& name,
                                                       const std::vector<URI>& regions )
{
  RDM::FaceTerm::Ptr term = build_component_abstract_type<RDM::FaceTerm>(type,name);

  m_face_terms->append(term);

  term->configure_option("regions" , regions);

  term->configure_option( RDM::Tags::mesh(), m_mesh.lock()->uri());
  term->configure_option( RDM::Tags::solver() , m_solver.lock()->uri());
  term->configure_option( RDM::Tags::physical_model() , m_physical_model.lock()->uri());

  return *term;
}

void DomainDiscretization::signal_create_cell_term( SignalArgs& args )
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

  create_cell_term( type, name, regions );
}


void DomainDiscretization::signal_create_face_term( SignalArgs& args )
{
  SignalOptions options( args );

  std::string name = options.value<std::string>("name");
  std::string type = options.value<std::string>("type");

  RDM::FaceTerm::Ptr term = build_component_abstract_type<RDM::FaceTerm>(type,name);

  m_face_terms->append(term);

  // configure the regions
  // if user did not specify, then use the whole topology (all regions)

  std::vector<URI> regions;
  if( options.check("regions") )
    regions = options.array<URI>("regions");
  else
    regions.push_back(mesh().topology().uri());

  create_face_term( type, name, regions );
}


void DomainDiscretization::signature_signal_create_cell_term( SignalArgs& args )
{
  SignalOptions options( args );

  // name

  options.add_option< OptionT<std::string> >("name", std::string() )
      ->description("Name for created volume term");

  // type

  /// @todo loop over the existing CellTerm providers to provide the available list

  //  std::vector< std::string > restricted;
  //  restricted.push_back( std::string("cf3.RDM.BcDirichlet") );
  //  XmlNode type_node = options.add_option< OptionT<std::string> >("Type", std::string("cf3.RDM.BcDirichlet"), "Type for created boundary");
  //  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted volume regions

  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->description("Regions where to apply the domain term");
}


void DomainDiscretization::signature_signal_create_face_term( SignalArgs& args )
{
  SignalOptions options( args );

  // name

  options.add_option< OptionT<std::string> >("name", std::string() )
      ->description("Name for created volume term");

  // type

  /// @todo loop over the existing FaceTerm providers to provide the available list

  //  std::vector< std::string > restricted;
  //  restricted.push_back( std::string("cf3.RDM.BcDirichlet") );
  //  XmlNode type_node = options.add_option< OptionT<std::string> >("Type", std::string("cf3.RDM.BcDirichlet"), "Type for created boundary");
  //  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted face regions

  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->description("Regions where to apply the domain term");
}

/////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3
