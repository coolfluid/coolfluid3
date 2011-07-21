// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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

#include "RDM/CellTerm.hpp"
#include "RDM/FaceTerm.hpp"

#include "DomainDiscretization.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;

namespace CF {
namespace RDM {


///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < DomainDiscretization, CAction, LibRDM > DomainDiscretization_Builder;

///////////////////////////////////////////////////////////////////////////////////////

DomainDiscretization::DomainDiscretization ( const std::string& name ) :
  CF::Solver::ActionDirector(name)
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


  m_face_terms = create_static_component_ptr<CActionDirector>("FaceTerms");
  m_cell_terms = create_static_component_ptr<CActionDirector>("CellTerms");
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

  std::string name = options.value<std::string>("Name");
  std::string type = options.value<std::string>("Type");

  // configure the regions
  // if user did not specify, then use the whole topology (all regions)

  std::vector<URI> regions;
  if( options.check("Regions") )
    regions = options.array<URI>("Regions");
  else
    regions.push_back(mesh().topology().uri());

  create_cell_term( type, name, regions );
}


void DomainDiscretization::signal_create_face_term( SignalArgs& args )
{
  SignalOptions options( args );

  std::string name = options.value<std::string>("Name");
  std::string type = options.value<std::string>("Type");

  RDM::FaceTerm::Ptr term = build_component_abstract_type<RDM::FaceTerm>(type,name);

  m_face_terms->append(term);

  // configure the regions
  // if user did not specify, then use the whole topology (all regions)

  std::vector<URI> regions;
  if( options.check("Regions") )
    regions = options.array<URI>("Regions");
  else
    regions.push_back(mesh().topology().uri());

  create_face_term( type, name, regions );
}


void DomainDiscretization::signature_signal_create_cell_term( SignalArgs& args )
{
  SignalOptions options( args );

  // name

  options.add_option< OptionT<std::string> >("Name", std::string() )
      ->set_description("Name for created volume term");

  // type

  /// @todo loop over the existing CellTerm providers to provide the available list

  //  std::vector< std::string > restricted;
  //  restricted.push_back( std::string("CF.RDM.BcDirichlet") );
  //  XmlNode type_node = options.add_option< OptionT<std::string> >("Type", std::string("CF.RDM.BcDirichlet"), "Type for created boundary");
  //  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted volume regions

  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->set_description("Regions where to apply the domain term");
}


void DomainDiscretization::signature_signal_create_face_term( SignalArgs& args )
{
  SignalOptions options( args );

  // name

  options.add_option< OptionT<std::string> >("Name", std::string() )
      ->set_description("Name for created volume term");

  // type

  /// @todo loop over the existing FaceTerm providers to provide the available list

  //  std::vector< std::string > restricted;
  //  restricted.push_back( std::string("CF.RDM.BcDirichlet") );
  //  XmlNode type_node = options.add_option< OptionT<std::string> >("Type", std::string("CF.RDM.BcDirichlet"), "Type for created boundary");
  //  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted face regions

  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->set_description("Regions where to apply the domain term");
}

/////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF
