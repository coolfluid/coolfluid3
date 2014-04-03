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

#include "ICNSImplicitToSemi.hpp"

namespace cf3
{

namespace UFEM
{

common::ComponentBuilder < ICNSImplicitToSemi, common::Action, LibUFEM > ICNSImplicitToSemi_Builder;

ICNSImplicitToSemi::ICNSImplicitToSemi(const std::string& name) :
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

ICNSImplicitToSemi::~ICNSImplicitToSemi()
{
}

void ICNSImplicitToSemi::execute()
{
  Handle<mesh::Mesh> source_mesh = options().value< Handle<mesh::Mesh> >("source_mesh");
  if(is_null(source_mesh))
    throw common::SetupError(FromHere(), "source_mesh is not set for " + uri().path());
  
  Handle<mesh::Mesh> destination_mesh = options().value< Handle<mesh::Mesh> >("destination_mesh");
  if(is_null(destination_mesh))
    throw common::SetupError(FromHere(), "destination_mesh is not set for " + uri().path());
  
  Handle<mesh::Field> destination_velocities(destination_mesh->geometry_fields().get_child("navier_stokes_u_solution"));
  if(is_null(destination_velocities))
    throw common::SetupError(FromHere(), "No field navier_stokes_u_solution in " + destination_mesh->uri().path());
  
  Handle<mesh::Field> destination_pressures(destination_mesh->geometry_fields().get_child("navier_stokes_p_solution"));
  if(is_null(destination_pressures))
    throw common::SetupError(FromHere(), "No field navier_stokes_p_solution in " + destination_mesh->uri().path());
  
  Handle<mesh::Field> source_field(source_mesh->geometry_fields().get_child("navier_stokes_solution"));
  if(is_null(source_field))
    throw common::SetupError(FromHere(), "No field navier_stokes_solution in " + source_mesh->uri().path());
  
  if(destination_velocities->size() != source_field->size())
    throw common::SetupError(FromHere(), "Source field size " + common::to_str(source_field->size()) + " does not match target size " + common::to_str(destination_velocities->size()));
  
  const Uint nb_rows = destination_velocities->size();
  const Uint dim = destination_velocities->row_size();
  
  if(source_field->row_size() != dim+1)
    throw common::SetupError(FromHere(), "Source field size incorrect: expected " + common::to_str(dim+1) + ", got " + common::to_str(source_field->row_size()));
  
  mesh::Field::ArrayT& destination_velocities_array = destination_velocities->array();
  mesh::Field::ArrayT& destination_pressures_array = destination_pressures->array();
  const mesh::Field::ArrayT& source_array = source_field->array();
  for(Uint i = 0; i != nb_rows; ++i)
  {
    const mesh::Field::ConstRow source_row = source_array[i];
    
    std::copy(source_row.begin(), source_row.begin()+dim, destination_velocities_array[i].begin());
    destination_pressures_array[i][0] = source_row[dim];
  }
}


} // namespace UFEM

} // namespace cf3
