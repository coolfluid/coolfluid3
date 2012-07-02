// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/AInterpolator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////

AInterpolator::AInterpolator(const std::string &name) : Component(name)
{
  regist_signal ( "interpolate" )
      .description( "Interpolate to given coordinates, not mesh-related" )
      .pretty_name( "Interpolate" )
      .connect   ( boost::bind ( &AInterpolator::signal_interpolate,    this, _1 ) )
      .signature ( boost::bind ( &AInterpolator::signature_interpolate, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void AInterpolator::interpolate_vars(const Field& source_field, Field& target_field, const std::vector<Uint>& source_vars, const std::vector<Uint>& target_vars)
{
  interpolate_vars(source_field,target_field.coordinates(),target_field,source_vars,target_vars);
}

////////////////////////////////////////////////////////////////////////////////

void AInterpolator::interpolate(const Field& source_field, const common::Table<Real>& target_coords, common::Table<Real>& target)
{
  if (target.row_size() != source_field.row_size())
    throw InvalidStructure(FromHere(), "Source field and Target field don't have matching variables");

  std::vector<Uint> source_vars(source_field.row_size());
  for (Uint i=0; i<source_vars.size(); ++i)
  {
    source_vars[i] = i;
  }
  std::vector<Uint> target_vars = source_vars;

  interpolate_vars(source_field,target_coords,target,source_vars,target_vars);
}

////////////////////////////////////////////////////////////////////////////////

void AInterpolator::interpolate(const Field& source_field, Field& target_field)
{
  interpolate(source_field,target_field.coordinates(),target_field);
}

////////////////////////////////////////////////////////////////////////////////

void AInterpolator::signal_interpolate ( common::SignalArgs& node )
{
  common::XML::SignalOptions options( node );

  URI source_uri = options.value<URI>("source");
  URI target_uri = options.value<URI>("target");
  URI coordinates_uri = options.value<URI>("coordinates");

  Field& source = *Handle<Field>(Core::instance().root().access_component(source_uri));
  common::Table<Real>& target = *Handle< common::Table<Real> >(Core::instance().root().access_component(target_uri));

  Handle< common::Table<Real> > coordinates;
  if (coordinates_uri.string() == URI().string())
  {
    if ( Handle< Field > target_field = Handle<Field>(target.handle<Component>()) )
    {
      coordinates = Handle< common::Table<Real> >(target_field->dict().coordinates().handle<Component>());
    }
    else
    {
      throw SetupError(FromHere(),"Argument \"coordinates\" not passed to interpolate() in "+uri().string());
    }
  }
  else
  {
    coordinates = Handle< common::Table<Real> >(Core::instance().root().access_component(coordinates_uri));
  }

  std::vector<Uint> source_vars = options.array<Uint>("source_vars");
  std::vector<Uint> target_vars = options.array<Uint>("target_vars");
  if (source_vars.size())
  {
    interpolate_vars(source,*coordinates,target,source_vars,target_vars);
  }
  else
  {
    interpolate(source,*coordinates,target);
  }
}

//////////////////////////////////////////////////////////////////////////////

void AInterpolator::signature_interpolate ( common::SignalArgs& node)
{
  common::XML::SignalOptions options( node );

  options.add("source",URI())
      .supported_protocol( URI::Scheme::CPATH )
      .description("Source field");

  options.add("target",URI())
      .supported_protocol( URI::Scheme::CPATH )
      .description("Target field or table");

  options.add("coordinates",URI())
      .supported_protocol( URI::Scheme::CPATH )
      .description("Table of coordinates if target is not a field");

  options.add("source_vars",std::vector<Uint>())
      .description("Source variable indices to interpolate");

  options.add("target_vars",std::vector<Uint>())
      .description("Target variable indices to interpolate to");
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
