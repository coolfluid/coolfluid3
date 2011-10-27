// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/Foreach.hpp"
#include "common/Link.hpp"
#include "common/FindComponents.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Field.hpp"
#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/FieldGroup.hpp"

#include "physics/PhysModel.hpp"

#include "RDM/RDSolver.hpp"

#include "SetupMultipleSolutions.hpp"


using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::mesh;

namespace cf3 {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SetupMultipleSolutions, common::Action, LibRDM > SetupMultipleSolutions_Builder;

///////////////////////////////////////////////////////////////////////////////////////

SetupMultipleSolutions::SetupMultipleSolutions ( const std::string& name ) : cf3::solver::Action(name)
{
  // options

  m_options.add_option< OptionT<Uint> >( "nb_levels", 1u )
      ->description("Number of solution levels to be created")
      ->pretty_name("Number of levels");
}

void SetupMultipleSolutions::execute()
{
  RDM::RDSolver& mysolver = solver().as_type< RDM::RDSolver >();

  /* nb_levels == rkorder */

  const Uint nb_levels = option("nb_levels").value<Uint>();

  Mesh&  mesh = *m_mesh.lock();
  Group& fields = mysolver.fields();

  // get the geometry field group

  FieldGroup& geometry = mesh.geometry_fields();

  const std::string solution_space = mysolver.option("solution_space").value<std::string>();

  // check that the geometry belongs to the same space as selected by the user

  FieldGroup::Ptr solution_group;

  if( solution_space == geometry.space() )
    solution_group = geometry.as_ptr<FieldGroup>();
  else
  {
    // check if solution space already exists
    solution_group = find_component_ptr_with_name<FieldGroup>( mesh, RDM::Tags::solution() );
    if ( is_null(solution_group) )
    {
      solution_group = mesh.create_space_and_field_group( RDM::Tags::solution(), FieldGroup::Basis::POINT_BASED, "cf3.mesh."+solution_space).as_ptr<FieldGroup>();
    }
    else // not null so check that space is what user wants
    {
      if( solution_space != solution_group->space() )
        throw NotImplemented( FromHere(), "Changing solution space not supported" );
    }
  }

  solution_group->add_tag( solution_space );

  // construct vector of variables

  const Uint nbdofs = physical_model().neqs();

  std::string vars;
  for(Uint i = 0; i < nbdofs; ++i)
  {
   vars += "u" + to_str(i) + "[1]";
   if( i != nbdofs-1 ) vars += ",";
  }

  // configure 1st solution

  Field::Ptr solution = find_component_ptr_with_tag<Field>( *solution_group, RDM::Tags::solution() );
  if ( is_null( solution ) )
  {
    solution = solution_group->create_field( RDM::Tags::solution(), vars ).as_ptr<Field>();

    solution->add_tag(Tags::solution());
  }

  // create the other solutions based on the first solution field

  std::vector< Field::Ptr > rk_steps;

  rk_steps.push_back(solution);

  for(Uint step = 1; step < nb_levels; ++step)
  {
    Field::Ptr solution_k = find_component_ptr_with_tag<Field>( *solution_group, RDM::Tags::solution() + to_str(step));
    if ( is_null( solution_k ) )
    {
      std::string name = std::string(Tags::solution()) + to_str(step);
      solution_k = solution_group->create_field( name, solution->descriptor().description() ).as_ptr<Field>();
      solution_k->descriptor().prefix_variable_names("rk" + to_str(step) + "_");
      solution_k->add_tag("rksteps");
    }

    cf3_assert( solution_k );
    rk_steps.push_back(solution_k);
  }

  // configure residual

  Field::Ptr residual = find_component_ptr_with_tag<Field>( *solution_group, RDM::Tags::residual());
  if ( is_null( residual ) )
  {
    residual = solution_group->create_field(Tags::residual(), solution->descriptor().description() ).as_ptr<Field>();
    residual->descriptor().prefix_variable_names("rhs_");
    residual->add_tag(Tags::residual());
  }


  // configure wave_speed

  Field::Ptr wave_speed = find_component_ptr_with_tag<Field>( *solution_group, RDM::Tags::wave_speed());
  if ( is_null( wave_speed ) )
  {
    wave_speed = solution_group->create_field(Tags::wave_speed(), "ws[1]" ).as_ptr<Field>();
    wave_speed->add_tag(Tags::wave_speed());
  }


  // create links

  if( ! fields.get_child_ptr( solution->name() ) )
    fields.create_component<Link>( solution->name() ).link_to(solution).add_tag(RDM::Tags::solution());
  if( ! fields.get_child_ptr( RDM::Tags::residual() ) )
    fields.create_component<Link>( RDM::Tags::residual() ).link_to(residual).add_tag(RDM::Tags::residual());
  if( ! fields.get_child_ptr( RDM::Tags::wave_speed() ) )
    fields.create_component<Link>( RDM::Tags::wave_speed() ).link_to(wave_speed).add_tag(RDM::Tags::wave_speed());

  for( Uint step = 1; step < rk_steps.size(); ++step)
  {
    if( ! fields.get_child_ptr( rk_steps[step]->name() ) )
      fields.create_component<Link>( rk_steps[step]->name() ).link_to( rk_steps[step] ).add_tag("rksteps");
  }


  // parallelize the solution if not yet done

  CommPattern& pattern = solution->parallelize();

  std::vector<URI> parallel_fields;
  parallel_fields.push_back( solution->uri() );

  for(Uint step = 1; step < nb_levels; ++step)
  {
    rk_steps[step]->parallelize_with( pattern );
    parallel_fields.push_back( rk_steps[step]->uri() );
  }

  mysolver.actions().get_child("Synchronize").configure_option("Fields", parallel_fields);

//  std::cout << "solution " << solution->uri().string() << std::endl;
//  std::cout << "residual " << residual->uri().string() << std::endl;
//  std::cout << "wave_speed " << wave_speed->uri().string() << std::endl;

}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
