// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/date_time/gregorian/gregorian.hpp>

#include "Common/CBuilder.hpp"

#include "Mesh/MeshMetadata.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;

ComponentBuilder<MeshMetadata, Component, LibMesh> MeshMetadata_Builder;

////////////////////////////////////////////////////////////////////////////////

MeshMetadata::MeshMetadata(const std::string& name) :
  Common::Component(name)
{
  // get the day of today
  m_properties["date"] = boost::gregorian::to_iso_extended_string(boost::gregorian::day_clock::local_day());
  // to convert back:
  // boost::gregorian::date date = boost::gregorian::from_simple_string(options()["date"].value_str());

  m_properties["time"] = 0.;
  m_properties["iter"] = 0u;
}


////////////////////////////////////////////////////////////////////////////////

boost::any& MeshMetadata::operator[](const std::string& name)
{
  return  property(name);
}

////////////////////////////////////////////////////////////////////////////////

const boost::any& MeshMetadata::operator[](const std::string& name) const
{
  return property(name);
}

////////////////////////////////////////////////////////////////////////////////

bool MeshMetadata::check(const std::string& name) const
{
  return properties().check(name);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
