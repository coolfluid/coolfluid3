// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionArray.hpp"
#include "common/Foreach.hpp"
#include "common/Link.hpp"
#include "common/FindComponents.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Mesh.hpp"

#include "physics/PhysModel.hpp"

#include "RDM/RDSolver.hpp"

#include "SetupSingleSolution.hpp"


using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SetupSingleSolution, common::Action, LibRDM > SetupSingleSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////

SetupSingleSolution::SetupSingleSolution ( const std::string& name ) : cf3::solver::Action(name)
{
}

void SetupSingleSolution::execute()
{
  RDM::RDSolver& mysolver = solver().as_type< RDM::RDSolver >();

  if(m_mesh.expired())
    throw SetupError(FromHere(), "SetupSingleSolution has no configured mesh in [" + uri().string() + "]" );

  Mesh& mesh = *m_mesh.lock();

  Group& fields = mysolver.fields();

  const Uint nbdofs = physical_model().neqs();

  // get the geometry field group

  SpaceFields& geometry = mesh.geometry_fields();

  const std::string solution_space = mysolver.option("solution_space").value<std::string>();

  // check that the geometry belongs to the same space as selected by the user

  SpaceFields::Ptr solution_group;

  if( solution_space == geometry.space() )
    solution_group = geometry.as_ptr<SpaceFields>();
  else
  {
    // check if solution space already exists
    solution_group = find_component_ptr_with_name<SpaceFields>( mesh, RDM::Tags::solution() );
    if ( is_null(solution_group) )
    {
      solution_group = mesh.create_space_and_field_group( RDM::Tags::solution(), SpaceFields::Basis::POINT_BASED, "cf3.mesh."+solution_space).as_ptr<SpaceFields>();
    }
    else // not null so check that space is what user wants
    {
      if( solution_space != solution_group->space() )
        throw NotImplemented( FromHere(), "Changing solution space not supported" );
    }
  }

  solution_group->add_tag( solution_space );

  // configure solution

  Field::Ptr solution = find_component_ptr_with_tag<Field>( *solution_group, RDM::Tags::solution() );
  if ( is_null( solution ) )
  {
    std::string vars;
    for(Uint i = 0; i < nbdofs; ++i)
    {
     vars += "u" + to_str(i) + "[1]";
     if( i != nbdofs-1 ) vars += ",";
    }

    solution = solution_group->create_field( RDM::Tags::solution(), vars ).as_ptr<Field>();

    solution->add_tag(RDM::Tags::solution());
  }

  /// @todo here we should check if space() order is correct,
  ///       if not the change space() by enriching or other appropriate action

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
    wave_speed = solution_group->create_field( Tags::wave_speed(), "ws[1]" ).as_ptr<Field>();
    wave_speed->add_tag(Tags::wave_speed());
  }

  // place link to the fields in the Fields group

  if( ! fields.get_child_ptr( RDM::Tags::solution() ) )
    fields.create_component<Link>( RDM::Tags::solution()   ).link_to(solution).add_tag(RDM::Tags::solution());
  if( ! fields.get_child_ptr( RDM::Tags::residual() ) )
    fields.create_component<Link>( RDM::Tags::residual()   ).link_to(residual).add_tag(RDM::Tags::residual());
  if( ! fields.get_child_ptr( RDM::Tags::wave_speed() ) )
    fields.create_component<Link>( RDM::Tags::wave_speed() ).link_to(wave_speed).add_tag(RDM::Tags::wave_speed());


  /// @todo apply here the bubble insertion if needed

  // parallelize the solution if not yet done

  solution->parallelize();

  std::vector<URI> sync_fields;
  sync_fields.push_back( solution->uri() );
  mysolver.actions().get_child("Synchronize").configure_option("Fields", sync_fields);

}

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3
