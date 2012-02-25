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
      .mark_basic();

  // Path to the mesh to write
  options().add_option("mesh", m_mesh)
      .description("Mesh to write")
      .pretty_name("Mesh")
      .link_to(&m_mesh)
      .mark_basic();

  // Output file path
  m_file_path = URI("mesh", URI::Scheme::FILE);
  options().add_option("file", m_file_path)
      .supported_protocol(URI::Scheme::FILE)  
      .description("File to write")
      .pretty_name("File")
      .link_to(&m_file_path)
      .mark_basic();

  // Regions to write. Default is entire mesh.
  options().add_option("regions", std::vector<URI>(1,"./"+std::string(Tags::topology())))
      .pretty_name("Regions")
      .description("Regions to write. Default is entire mesh. URI can be relative to mesh")
      .mark_basic();

  // Option to enable overlap
  m_enable_overlap = false;
  options().add_option("enable_overlap", m_enable_overlap)
      .pretty_name("Enable Overlap")
      .description("Includes ghost cells")
      .mark_basic()
      .link_to(&m_enable_overlap);

  // Option to enable surfaces such as boundaries
  m_region_filter  .enable_surfaces = true;   // default value
  m_entities_filter.enable_surfaces = true; // default value
  options().add_option("enable_surfaces", m_region_filter.enable_surfaces)
      .pretty_name("Enable Surfaces")
      .description("Includes interior faces in mesh")
      .mark_basic()
      .link_to(&m_region_filter  .enable_surfaces)
      .link_to(&m_entities_filter.enable_surfaces);

  // Option to enable interior cells
  m_region_filter  .enable_interior_cells = true; // default value
  m_entities_filter.enable_interior_cells = true; // default value
  options().add_option("enable_interior_cells", m_region_filter.enable_interior_cells)
      .pretty_name("Enable Interior Cells")
      .description("Includes interior cells in mesh")
      .mark_basic()
      .link_to(&m_region_filter  .enable_interior_cells)
      .link_to(&m_entities_filter.enable_interior_cells);

  // Option to enable interior faces
  m_region_filter  .enable_interior_faces = false; // default value
  m_entities_filter.enable_interior_faces = false; // default value
  options().add_option("enable_interior_faces", m_region_filter.enable_interior_faces)
      .pretty_name("Enable Interior Faces")
      .description("Includes interior faces in mesh")
      .mark_basic()
      .link_to(&m_region_filter  .enable_interior_faces)
      .link_to(&m_entities_filter.enable_interior_faces);
}

////////////////////////////////////////////////////////////////////////////////

void MeshWriter::config_fields()
{
  if (is_null(m_mesh))
    throw SetupError(FromHere(),"Mesh was not configured in mesh-writer ["+uri().string()+"]");

  std::vector<URI> field_uris = options()["fields"].value< std::vector<URI> >();
  m_fields.clear();
  m_fields.reserve(field_uris.size());
  boost_foreach ( const URI& uri, field_uris)
  {
    m_fields.push_back(Handle<Field const>(m_mesh->access_component_checked(uri)));
    if ( is_null(m_fields.back()) )
      throw ValueNotFound(FromHere(),"Invalid type of field URI ["+uri.string()+"]");
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshWriter::config_regions()
{
  if (is_null(m_mesh))
    throw SetupError(FromHere(),"Mesh was not configured in mesh-writer ["+uri().string()+"]");

  std::vector<URI> region_uris = options()["regions"].value< std::vector<URI> >();
  cf3_assert(region_uris.size());
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

MeshWriter::~MeshWriter()
{
}

//////////////////////////////////////////////////////////////////////////////

void MeshWriter::execute()
{
  // Check if the mesh was configured
  if (is_null(m_mesh))
    throw SetupError(FromHere(),"Mesh was not configured in mesh-writer ["+uri().string()+"]");

  CFinfo << "Writing mesh " << m_file_path << CFendl;

  // Configure the fields to write
  config_fields();

  // Configure the regions to write
  config_regions();

  m_filtered_entities.clear();
  boost_foreach(const Handle<Region const>& region, m_regions)
    boost_foreach(const Entities& entities, find_components_recursively_with_filter<Entities>(*region,m_entities_filter))
      m_filtered_entities.push_back(entities.handle<Entities>());

  // Call implementation
  write();
}

//////////////////////////////////////////////////////////////////////////////

void MeshWriter::write_from_to(const Mesh& mesh, const URI& file_path)
{
  options().configure_option("mesh",mesh.handle<Mesh const>());
  options().configure_option("file",file_path);
  execute();
};


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
