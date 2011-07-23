// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"

#include "RDM/RDSolver.hpp"

#include "SetupSingleSolution.hpp"

/////////////////////////////////////////////////////////////////////////////////////

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

  // configure solution

  CField::Ptr solution = find_component_ptr_with_tag<CField>( mesh, RDM::Tags::solution() );
  if ( is_null( solution ) )
  {
    std::string vars;
    for(Uint i = 0; i < nbdofs; ++i)
    {
     vars += "u" + to_str(i) + "[1]";
     if( i != nbdofs-1 ) vars += ",";
    }

    solution =
        mesh.create_field( RDM::Tags::solution(),CField::Basis::POINT_BASED,"space[0]",vars).as_ptr<CField>();

    solution->add_tag(RDM::Tags::solution());
  }

  /// @todo here we should check if space() order is correct,
  ///       if not the change space() by enriching or other appropriate action

  // configure residual

  CField::Ptr residual = find_component_ptr_with_tag<CField>( mesh, RDM::Tags::residual());
  if ( is_null( residual ) )
  {
    residual = mesh.create_field(Tags::residual(), *solution ).as_ptr<CField>();
    residual->add_tag(Tags::residual());
  }

  // configure wave_speed

  CField::Ptr wave_speed = find_component_ptr_with_tag<CField>( mesh, RDM::Tags::wave_speed());
  if ( is_null( wave_speed ) )
  {
    wave_speed = mesh.create_scalar_field(Tags::wave_speed(), *solution).as_ptr<CField>();
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

  /// @todo fix parallelization is not working

//  solution->parallelize();

}

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF
