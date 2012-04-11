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
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Field.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Solver.hpp"
#include "solver/actions/ForAllCells.hpp"
#include "solver/actions/ForAllFaces.hpp"

#include "sdm/LibSDM.hpp"
#include "sdm/BoundaryConditions.hpp"
#include "sdm/BC.hpp"
#include "sdm/Tags.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BoundaryConditions, common::Action, LibSDM > BoundaryConditions_Builder;

///////////////////////////////////////////////////////////////////////////////////////

BoundaryConditions::BoundaryConditions ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

  // signals

  regist_signal( "create_boundary_condition" )
      .connect  ( boost::bind( &BoundaryConditions::signal_create_boundary_condition, this, _1 ) )
      .signature( boost::bind( &BoundaryConditions::signature_signal_create_boundary_condition, this, _1))
      .description("creates a boundary condition")
      .pretty_name("Create Boundary Condition");

  m_bcs = create_static_component<ActionDirector>("BCs");
}

void BoundaryConditions::execute()
{
  CFdebug << "BoundaryConditions EXECUTE" << CFendl;
  foreach_container( (const Handle<Region const>& region) (Handle<BC>& bc), m_bc_per_region)
  {
    if (region)
    {
      boost_foreach( const Entities& faces, find_components_recursively_with_tag<Entities>(*region,mesh::Tags::face_entity()) )
      {
        bc->set_face_entities(faces);
        CFdebug << "BoundaryConditions: executing " << bc->name() << " for cells " << faces.uri() << CFendl;
        for (Uint face_idx=0; face_idx<faces.size(); ++face_idx)
        {
          bc->set_face_element(face_idx);
          bc->execute();
          bc->unset_face_element();
        }
      }
    }
  }
}

BC& BoundaryConditions::create_boundary_condition( const std::string& type,
                                                   const std::string& name,
                                                   const std::vector<URI>& regions )
{
  Handle< BC > bc = m_bcs->create_component<BC>(name, type);

  bc->options().configure_option( sdm::Tags::solver(),         solver().handle<Component>());
  bc->options().configure_option( sdm::Tags::mesh(),           mesh().handle<Component>());

  if (regions.size() == 0)
    bc->options().configure_option("regions", solver().options().option("regions").value< std::vector<common::URI> >());
  else
    bc->options().configure_option("regions", regions);

  bc->options().configure_option( sdm::Tags::physical_model(), physical_model().handle<Component>());

  bc->initialize();

  boost_foreach(const URI& region_uri, regions)
  {
    m_bc_per_region[access_component(region_uri)->handle<Region>()] = bc->handle<BC>();
  }

  CFinfo << "Created BC   " << name << "(" << type << ") for regions " << CFendl;
  const std::string regions_option_name("regions");
  boost_foreach(const URI& region_uri, bc->options().option(regions_option_name).value< std::vector<URI> >())
  {
    CFinfo << "    - " << access_component(region_uri)->uri().path() << CFendl;
  }

  return *bc;
}

void BoundaryConditions::signal_create_boundary_condition( SignalArgs& args )
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

  BC& created_component = create_boundary_condition( type, name, regions );

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("created_component", created_component.uri());

}


void BoundaryConditions::signature_signal_create_boundary_condition( SignalArgs& args )
{
  SignalOptions options( args );

  // name

  options.add_option("name", std::string() )
      .description("Name for created term");

  // type

  /// @todo loop over the existing CellTerm providers to provide the available list

  options.add_option("type", std::string("cf3.sdm.Convection"))
      .description("Type for created term");

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted volume regions

  options.add_option("regions", dummy )
      .description("Regions where to apply the term");
}

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3
