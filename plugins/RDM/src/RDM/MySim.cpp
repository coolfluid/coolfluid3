// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "boost/assign/list_of.hpp"

#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/FindComponents.hpp"


#include "common/XML/SignalOptions.hpp"

#include "mesh/LoadMesh.hpp"
#include "mesh/Cells.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/Domain.hpp"
#include "mesh/Region.hpp"

#include "physics/PhysModel.hpp"

#include "solver/ModelSteady.hpp"
#include "solver/Solver.hpp"

#include "RDM/SteadyExplicit.hpp"
#include "RDM/MySim.hpp"
#include "RDM/RDSolver.hpp"
#include "RDM/BoundaryConditions.hpp"
#include "RDM/InitialConditions.hpp"
#include "RDM/DomainDiscretization.hpp"
#include "RDM/CellTerm.hpp"

namespace cf3 {
namespace RDM {

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::physics;
using namespace cf3::solver;

common::ComponentBuilder < MySim, solver::Wizard, LibRDM > MySim_Builder;

////////////////////////////////////////////////////////////////////////////////

MySim::MySim ( const std::string& name  ) :
  solver::Wizard ( name )
{
  // signals

  regist_signal( "create_model" )
    .connect( boost::bind( &MySim::signal_create_model, this, _1 ) )
    .description("Creates a scalar advection model")
    .pretty_name("Create Model");

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);

  signal("create_model")->signature( boost::bind( &MySim::signature_create_model, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

MySim::~MySim()
{
}

////////////////////////////////////////////////////////////////////////////////

void MySim::signal_create_model ( common::SignalArgs& node )
{
  SignalOptions options( node );

  SteadyExplicit& wizard = *create_component<SteadyExplicit>("wizard");

  std::string name  = options.value<std::string>("model_name");

  Model& model = wizard.create_model(name, "Scalar2D");

  Handle<Domain>     domain(model.get_child("Domain"));
  Handle<RDM::RDSolver> solver(model.get_child("Solver"));

  // load the mesh
  {
    SignalFrame frame;
    SignalOptions options( frame );

    std::vector<URI> files;

    URI file( "file:rectangle2x1-tg-p1-953.msh");
  //  URI file( "file:rectangle2x1-tg-p2-3689.msh");

  //  URI file( "file:rectangle2x1-qd-p1-861.msh");
  //  URI file( "file:rectangle2x1-qd-p2-3321.msh");

    options.add("file", file );
    options.add("name", std::string("Mesh") );

    domain->signal_load_mesh( frame );
  }

  // setup solver
  {
    solver->get_child("IterativeSolver")->configure_option_recursively("MaxIter", 2250u);
  }

  // boudnary term
  {
    SignalFrame frame;
    SignalOptions options( frame );

    std::vector<URI> regions;
    boost_foreach( const Region& region, find_components_recursively_with_name<Region>(*domain,"bottom"))
      regions.push_back( region.uri() );
    boost_foreach( const Region& region, find_components_recursively_with_name<Region>(*domain,"left"))
      regions.push_back( region.uri() );

    cf3_assert( regions.size() == 2u);

    std::string name ("WEAK_INLET");

    options.add("Name",name);
    options.add("Type","cf3.RDM.BcDirichlet");
    options.add("Regions", regions);

    solver->boundary_conditions().signal_create_boundary_condition(frame);

    Handle< Component > inletbc = find_component_ptr_recursively_with_name( *solver, name );
    cf3_assert( is_not_null(inletbc) );

    std::vector<std::string> fns;
    fns.push_back("if(x>=-1.4,if(x<=-0.6,0.5*(cos(3.141592*(x+1.0)/0.4)+1.0),0.),0.)");

    inletbc->options().set("functions", fns);
  }

  // initialization
  {
    SignalFrame frame;
    SignalOptions options( frame );

    std::vector<std::string> functions(1);
    functions[0] = "0.";
    options.add("functions", functions);

    solver->initial_conditions().signal_create_initial_condition( frame );
  }

  // LDA scheme
  {
    CFinfo << "solving with LDA scheme" << CFendl;

    // delete previous domain terms
    Component& domain_terms = *solver->get_child("compute_domain_terms");
    boost_foreach( RDM::CellTerm& term, find_components_recursively<RDM::CellTerm>( domain_terms ))
    {
      const std::string name = term.name();
      domain_terms.remove_component( name );
    }

    cf3_assert( domain_terms.count_children() == 0 );

    Mesh& mesh = find_component<Mesh>(*domain);

    SignalFrame frame;
    SignalOptions options( frame );

    std::vector<URI> regions;
    boost_foreach( const Region& region, find_components_recursively_with_name<Region>(mesh,"topology"))
      regions.push_back( region.uri() );

    cf3_assert( regions.size() == 1u);

    options.add("Name","INTERNAL");
    options.add("Type","cf3.RDM.Schemes.LDA");
    options.add("Regions", regions);

    solver->domain_discretization().signal_create_cell_term(frame);

    // solver->solve();
  }

}

////////////////////////////////////////////////////////////////////////////////

void MySim::signature_create_model( SignalArgs& node )
{
  SignalOptions options( node );

  options.add("model_name", std::string())
      .description("Name for created model");
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
