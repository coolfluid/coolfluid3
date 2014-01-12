// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include <mesh/Mesh.hpp>

#include "ICNSSemiToImplicit.hpp"

namespace cf3
{

namespace UFEM
{

common::ComponentBuilder < ICNSSemiToImplicit, common::Action, LibUFEM > ICNSSemiToImplicit_Builder;

ICNSSemiToImplicit::ICNSSemiToImplicit(const std::string& name) :
  common::Action(name)
{
  options().add("source_mesh", Handle<mesh::Mesh>())
    .pretty_name("Source Mesh")
    .description("Mesh to copy from")
    .mark_basic();
    
  options().add("destination_mesh", Handle<mesh::Mesh>())
    .pretty_name("Destination Mesh")
    .description("Mesh to copy to")
    .mark_basic();
}

ICNSSemiToImplicit::~ICNSSemiToImplicit()
{
}

void ICNSSemiToImplicit::execute()
{
  Handle<mesh::Mesh> source_mesh = options().value< Handle<mesh::Mesh> >("source_mesh");
  if(is_null(source_mesh))
    throw common::SetupError(FromHere(), "source_mesh is not set for " + uri().path());
  
  Handle<mesh::Mesh> destination_mesh = options().value< Handle<mesh::Mesh> >("destination_mesh");
  if(is_null(destination_mesh))
    throw common::SetupError(FromHere(), "destination_mesh is not set for " + uri().path());
  
  Handle<mesh::Field> source_velocities(source_mesh->geometry_fields().get_child("navier_stokes_u_solution"));
  if(is_null(source_velocities))
    throw common::SetupError(FromHere(), "No field navier_stokes_u_solution in " + source_mesh->uri().path());
  
  Handle<mesh::Field> source_pressures(source_mesh->geometry_fields().get_child("navier_stokes_p_solution"));
  if(is_null(source_pressures))
    throw common::SetupError(FromHere(), "No field navier_stokes_p_solution in " + source_mesh->uri().path());
  
  Handle<mesh::Field> destination_field(destination_mesh->geometry_fields().get_child("navier_stokes_solution"));
  if(is_null(destination_field))
    throw common::SetupError(FromHere(), "No field navier_stokes_solution in " + destination_mesh->uri().path());
  
  if(source_velocities->size() != destination_field->size())
    throw common::SetupError(FromHere(), "Source field size " + common::to_str(source_velocities->size()) + " does not match target size " + common::to_str(destination_field->size()));
  
  const Uint nb_rows = source_velocities->size();
  const Uint dim = source_velocities->row_size();
  
  if(destination_field->row_size() != dim+1)
    throw common::SetupError(FromHere(), "Destination field size incorrect: expected " + common::to_str(dim+1) + ", got " + common::to_str(destination_field->row_size()));
  
  const mesh::Field::ArrayT& source_velocities_array = source_velocities->array();
  const mesh::Field::ArrayT& source_pressures_array = source_pressures->array();
  mesh::Field::ArrayT& destination_array = destination_field->array();
  for(Uint i = 0; i != nb_rows; ++i)
  {
    const mesh::Field::ConstRow velocities = source_velocities_array[i];
    mesh::Field::Row dest_row = destination_array[i];
    
    std::copy(velocities.begin(), velocities.end(), dest_row.begin());
    dest_row[dim] = source_pressures_array[i][0];
  }
}


} // namespace UFEM

} // namespace cf3
