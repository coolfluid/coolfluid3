// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"
#include "common/Environment.hpp"
#include "common/Core.hpp"
#include "common/FindComponents.hpp"

#include "mesh/MeshWriter.hpp"
#include "mesh/MeshMetadata.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/CellFaces.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

MeshWriter::MeshWriter ( const std::string& name  ) :
  Action ( name )
{
  mark_basic();

  // Fields to write
  options().add_option("fields",std::vector<URI>())
      .description("Fields to ouptut")
      .mark_basic()
      .attach_trigger( boost::bind( &MeshWriter::config_fields, this ) );

  // Path to the mesh to write
  options().add_option( "mesh", URI())
      .supported_protocol(URI::Scheme::CPATH)
      .description("Mesh to write")
      .pretty_name("Mesh")
      .mark_basic();

  // Output file path
  options().add_option("file", URI("mesh", URI::Scheme::FILE))
      .supported_protocol(URI::Scheme::FILE)  
      .description("File to write")
      .pretty_name("File")
      .mark_basic();

  // Regions to write. Default is entire mesh.
  options().add_option("regions", std::vector<URI>(1,"./"+std::string(Tags::topology())))
      .pretty_name("Regions")
      .description("Regions to write. Default is entire mesh. URI can be relative to mesh")
      .mark_basic();

  // Option to enable overlap
  options().add_option("overlap", false)
      .pretty_name("Enable Overlap")
      .description("Includes ghost cells")
      .mark_basic();

  // Option to enable surfaces such as boundaries
  m_region_filter  .enable_surfaces = true;   // default value
  m_entities_filter.enable_surfaces = true; // default value
  options().add_option("surfaces", m_region_filter.enable_surfaces)
      .pretty_name("Enable Surfaces")
      .description("Includes interior faces in mesh")
      .mark_basic()
      .link_to(&m_region_filter.enable_surfaces)
      .link_to(&m_entities_filter.enable_surfaces);

  // Option to enable interior cells
  m_region_filter  .enable_interior_cells   = true; // default value
  m_entities_filter.enable_interior_cells = true; // default value
  options().add_option("interior_cells", m_region_filter.enable_interior_cells)
      .pretty_name("Enable Interior Cells")
      .description("Includes interior cells in mesh")
      .mark_basic()
      .link_to(&m_region_filter.enable_interior_cells)
      .link_to(&m_entities_filter.enable_interior_cells);

  // Option to enable interior faces
  m_region_filter  .enable_interior_faces   = false; // default value
  m_entities_filter.enable_interior_faces = false; // default value
  options().add_option("interior_faces", m_region_filter.enable_interior_faces)
      .pretty_name("Enable Interior Faces")
      .description("Includes interior faces in mesh")
      .mark_basic()
      .link_to(&m_region_filter.enable_interior_faces)
      .link_to(&m_entities_filter.enable_interior_faces);
}

////////////////////////////////////////////////////////////////////////////////

void MeshWriter::config_fields()
{
  std::vector<URI> field_uris = options()["fields"].value< std::vector<URI> >();

  m_fields.clear();
  m_fields.reserve(field_uris.size());
  boost_foreach ( const URI& uri, field_uris)
  {
    m_fields.push_back(Handle<Field const>(access_component_checked(uri)));
    if ( is_null(m_fields.back()) )
      throw ValueNotFound(FromHere(),"Invalid type of field URI ["+uri.string()+"]");
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshWriter::config_regions()
{
  if (is_null(m_mesh))
    m_mesh = access_component(options().option("mesh").value<URI>())->handle<Mesh const>();
  if (is_null(m_mesh))
    throw ValueNotFound(FromHere(),"Invalid type of mesh URI ["+options().option("mesh").value<URI>().string()+"]");

  std::vector<URI> region_uris = options()["regions"].value< std::vector<URI> >();

  m_regions.clear();
  m_regions.reserve(region_uris.size());
  boost_foreach ( const URI& uri, region_uris)
  {
    m_regions.push_back(Handle<Region const>(m_mesh->access_component_checked(uri)));
    if ( is_null(m_regions.back()) )
      throw ValueNotFound(FromHere(),"Invalid URI ["+uri.string()+"]");
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshWriter::set_fields(const std::vector<Handle< Field > >& fields)
{
  CFwarn << "Deprecated function, configure using URI's" << CFendl;
  m_fields.clear();
  m_fields.reserve(fields.size());
  boost_foreach( Handle< Field > field, fields )
    m_fields.push_back(field);
}

////////////////////////////////////////////////////////////////////////////////

MeshWriter::~MeshWriter()
{
}

//////////////////////////////////////////////////////////////////////////////

void MeshWriter::execute()
{
  // Get the mesh
  m_mesh = access_component_checked(options().option("mesh").value<URI>())->handle<Mesh const>();
  if (is_null(m_mesh))
    throw ValueNotFound(FromHere(),"Invalid type of mesh URI ["+options().option("mesh").value<URI>().string()+"]");

  // Configure the fields to write
  config_fields();

  // Configure the regions to write
  config_regions();

  // Get the file path
  std::string file = options().option("file").value<URI>().string();

  // Call implementation
  write_from_to(*m_mesh,file);
}

//////////////////////////////////////////////////////////////////////////////

bool MeshWriter::RegionFilter::operator()(const Component& component)
{
  Uint cnt(0);

  // enable interior cells
  if (enable_interior_cells)
    cnt += common::count(common::find_components<Cells>(component));

  // enable surfaces
  if (enable_surfaces)
    cnt += common::count(common::find_components<Faces>(component));

  // enable interior faces
  if (enable_interior_faces)
    cnt += common::count(common::find_components<CellFaces>(component));

  return (cnt>0);
}

//////////////////////////////////////////////////////////////////////////////

bool MeshWriter::EntitiesFilter::operator()(const Component& component)
{
  // enable interior cells
  if (component.handle<Cells>() && enable_interior_cells)
    return true;

  // enable interior faces
  if (component.handle<CellFaces>() && enable_interior_faces)
    return true;

  // enable surfaces
  if (component.handle<Faces>() && enable_surfaces)
    return true;

  return false;
}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
