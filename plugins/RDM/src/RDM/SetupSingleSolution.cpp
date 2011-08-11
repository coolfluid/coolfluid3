// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/FindComponents.hpp"

#include "Math/VariablesDescriptor.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"

#include "RDM/RDSolver.hpp"

#include "SetupSingleSolution.hpp"


using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SetupSingleSolution, CAction, LibRDM > SetupSingleSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////

SetupSingleSolution::SetupSingleSolution ( const std::string& name ) : CF::Solver::Action(name)
{
}

void SetupSingleSolution::execute()
{
  RDM::RDSolver& mysolver = solver().as_type< RDM::RDSolver >();

  if(m_mesh.expired())
    throw SetupError(FromHere(), "SetupSingleSolution has no configured mesh in [" + uri().string() + "]" );

  CMesh& mesh = *m_mesh.lock();

  CGroup& fields = mysolver.fields();

  const Uint nbdofs = physical_model().neqs();

  // get the geometry field group

  Geometry& geometry = mesh.geometry();

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
      boost_foreach(CEntities& elements, mesh.topology().elements_range())
          elements.create_space( RDM::Tags::solution(), "CF.Mesh.SF.SF"+elements.element_type().shape_name() + solution_space );
      solution_group = mesh.create_field_group( RDM::Tags::solution(), FieldGroup::Basis::POINT_BASED).as_ptr<FieldGroup>();
    }
    else // not null so check that space is what user wants
    {
      if( solution_space != solution_group->space() )
        throw NotImplemented( FromHere(), "Changing solution space not supported" );
    }
  }

  // configure solution

  Field::Ptr solution = find_component_ptr_with_tag<Field>( geometry, RDM::Tags::solution() );
  if ( is_null( solution ) )
  {
    std::string vars;
    for(Uint i = 0; i < nbdofs; ++i)
    {
     vars += "u" + to_str(i) + "[1]";
     if( i != nbdofs-1 ) vars += ",";
    }

    solution =
        solution_group->create_field( RDM::Tags::solution(), vars ).as_ptr<Field>();

    solution->add_tag(RDM::Tags::solution());
  }

  /// @todo here we should check if space() order is correct,
  ///       if not the change space() by enriching or other appropriate action

  // configure residual

  Field::Ptr residual = find_component_ptr_with_tag<Field>( geometry, RDM::Tags::residual());
  if ( is_null( residual ) )
  {
    residual = solution_group->create_field(Tags::residual(), solution->descriptor().description() ).as_ptr<Field>();
    residual->descriptor().prefix_variable_names("rhs_");
    residual->add_tag(Tags::residual());
  }

  // configure wave_speed

  Field::Ptr wave_speed = find_component_ptr_with_tag<Field>( geometry, RDM::Tags::wave_speed());
  if ( is_null( wave_speed ) )
  {
    wave_speed = solution_group->create_field( Tags::wave_speed(), "ws[1]" ).as_ptr<Field>();
    wave_speed->add_tag(Tags::wave_speed());
  }

  // place link to the fields in the Fields group

  if( ! fields.get_child_ptr( RDM::Tags::solution() ) )
    fields.create_component<CLink>( RDM::Tags::solution()   ).link_to(solution).add_tag(RDM::Tags::solution());
  if( ! fields.get_child_ptr( RDM::Tags::residual() ) )
    fields.create_component<CLink>( RDM::Tags::residual()   ).link_to(residual).add_tag(RDM::Tags::residual());
  if( ! fields.get_child_ptr( RDM::Tags::wave_speed() ) )
    fields.create_component<CLink>( RDM::Tags::wave_speed() ).link_to(wave_speed).add_tag(RDM::Tags::wave_speed());


  /// @todo apply here the bubble insertion if needed

  // parallelize the solution if not yet done

  solution->parallelize();

  std::vector<URI> sync_fields;
  sync_fields.push_back( solution->uri() );
  mysolver.actions().get_child("Synchronize").configure_option("Fields", sync_fields);

}

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF
