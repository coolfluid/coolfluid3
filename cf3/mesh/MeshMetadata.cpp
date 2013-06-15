// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/date_time/gregorian/gregorian.hpp>

#include "common/Builder.hpp"
#include "common/PropertyList.hpp"

#include "mesh/MeshMetadata.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;

ComponentBuilder<MeshMetadata, Component, LibMesh> MeshMetadata_Builder;

////////////////////////////////////////////////////////////////////////////////

MeshMetadata::MeshMetadata(const std::string& name) :
  common::Component(name)
{
  // get the day of today
  properties()["date"] = boost::gregorian::to_iso_extended_string(boost::gregorian::day_clock::local_day());
  // to convert back:
  // boost::gregorian::date date = boost::gregorian::from_simple_string(options()["date"].value_str());

  properties()["time"] = 0.;
  properties()["iter"] = 0u;
}


////////////////////////////////////////////////////////////////////////////////

boost::any& MeshMetadata::operator[](const std::string& name)
{
  return  properties().property(name);
}

////////////////////////////////////////////////////////////////////////////////

const boost::any& MeshMetadata::operator[](const std::string& name) const
{
  return properties().property(name);
}

////////////////////////////////////////////////////////////////////////////////

bool MeshMetadata::check(const std::string& name) const
{
  return properties().check(name);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
