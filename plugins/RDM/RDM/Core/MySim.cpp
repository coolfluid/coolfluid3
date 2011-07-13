// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "boost/assign/list_of.hpp"

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/FindComponents.hpp"


#include "Common/XML/SignalOptions.hpp"

#include "Mesh/LoadMesh.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CRegion.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CModelSteady.hpp"
#include "Solver/CSolver.hpp"

#include "RDM/Core/MySim.hpp"
#include "RDM/Core/RKRD.hpp"
#include "RDM/Core/DomainTerm.hpp"

namespace CF {
namespace RDM {

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Physics;
using namespace CF::Solver;

Common::ComponentBuilder < MySim, Solver::CWizard, LibCore > MySim_Builder;

////////////////////////////////////////////////////////////////////////////////

MySim::MySim ( const std::string& name  ) :
  Solver::CWizard ( name )
{
  // signals

  this->regist_signal ( "create_model" , "Creates a scalar advection model", "Create Model" )->signal->connect ( boost::bind ( &MySim::signal_create_model, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;

  signal("create_model")->signature->connect( boost::bind( &MySim::signature_create_model, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

MySim::~MySim()
{
}

////////////////////////////////////////////////////////////////////////////////

void MySim::signal_create_model ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  std::string name  = options.value<std::string>("ModelName");

  CModel::Ptr model = Core::instance().root().create_component_ptr<CModelSteady>( name );

  // create domain

  CDomain& domain = model->create_domain( "Domain" );

  // create the Physical Model

  PhysModel::Ptr pm = build_component_abstract_type<PhysModel>( "Scalar2D", "Physics");
  pm->mark_basic();
  model->add_component( pm );

  // setup iterative solver

  CSolver::Ptr solver = build_component_abstract_type<CSolver>( LibCore::library_namespace() + ".RKRD", "Solver");
  solver->mark_basic();
  model->add_component( solver );

  solver->configure_option("physics", pm->uri() );

  // load the mesh
  {
    SignalFrame frame;
    SignalOptions options( frame );

    std::vector<URI> files;

    URI file( "file:rectangle2x1-tg-p1-953.msh");
  //  URI file( "file:rectangle2x1-tg-p2-3689.msh");

  //  URI file( "file:rectangle2x1-qd-p1-861.msh");
  //  URI file( "file:rectangle2x1-qd-p2-3321.msh");

    options.add_option<OptionURI>("file", file );
    options.add_option< OptionT<std::string> >("name", std::string("Mesh") );

    domain.signal_load_mesh( frame );
  }

  // setup solver
  {
    solver->configure_option("domain", domain.uri());
    solver->get_child("time_stepping").configure_option("cfl", 0.5);
    solver->get_child("time_stepping").configure_option("MaxIter", 2250u);
  }

  // boudnary term
  {
    SignalFrame frame;
    SignalOptions options( frame );

    std::vector<URI> regions;
    boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"bottom"))
      regions.push_back( region.uri() );
    boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"left"))
      regions.push_back( region.uri() );

    cf_assert( regions.size() == 2u);

    std::string name ("WEAK_INLET");

    options.add_option< OptionT<std::string> >("Name",name);
    options.add_option< OptionT<std::string> >("Type","CF.RDM.Core.BcDirichlet");
    options.add_option< OptionArrayT<URI> >("Regions", regions);

    solver->as_ptr<RKRD>()->signal_create_boundary_term(frame);

    Component::Ptr inletbc = find_component_ptr_recursively_with_name( *solver, name );
    cf_assert( is_not_null(inletbc) );

    std::vector<std::string> fns;
    fns.push_back("if(x>=-1.4,if(x<=-0.6,0.5*(cos(3.141592*(x+1.0)/0.4)+1.0),0.),0.)");
//    fns.push_back("cos(2*3.141592*(x+y))");

    inletbc->configure_option("functions", fns);
  }

  // initialization
  {
    SignalFrame frame;
    SignalOptions options( frame );

    std::vector<std::string> functions(1);
    functions[0] = "0.";
    options.add_option< OptionArrayT<std::string> >("functions", functions);

    solver->as_type<RKRD>().signal_initialize_solution( frame );
  }

  // LDA scheme
  {
    CFinfo << "solving with LDA scheme" << CFendl;

    // delete previous domain terms
    Component& domain_terms = solver->get_child("compute_domain_terms");
    boost_foreach( RDM::DomainTerm& term, find_components_recursively<RDM::DomainTerm>( domain_terms ))
    {
      const std::string name = term.name();
      domain_terms.remove_component( name );
    }

    cf_assert( domain_terms.count_children() == 0 );

    CMesh& mesh = find_component<CMesh>(domain);

    SignalFrame frame;
    SignalOptions options( frame );

    std::vector<URI> regions;
    boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(mesh,"topology"))
      regions.push_back( region.uri() );

    cf_assert( regions.size() == 1u);

    options.add_option< OptionT<std::string> >("Name","INTERNAL");
    options.add_option< OptionT<std::string> >("Type","CF.RDM.Schemes.CSysLDA");
    options.add_option< OptionArrayT<URI> >("Regions", regions);

    solver->as_ptr<RKRD>()->signal_create_domain_term(frame);

    // solver->solve();
  }

}

////////////////////////////////////////////////////////////////////////////////

void MySim::signature_create_model( SignalArgs& node )
{
  SignalOptions options( node );

  options.add_option< OptionT<std::string> >("ModelName", std::string())
      ->set_description("Name for created model");
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
