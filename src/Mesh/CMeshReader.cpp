// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BoostFilesystem.hpp"

#include "Common/Signal.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionURI.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/all_reduce.hpp"
#include "Common/MPI/operations.hpp"
#include "Common/CreateComponent.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CElements.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;

////////////////////////////////////////////////////////////////////////////////

CMeshReader::CMeshReader ( const std::string& name  ) :
  Component ( name )
{
  mark_basic();

  // signals
  this->regist_signal ( "read" , "reads a mesh", "Read mesh" )->signal->connect ( boost::bind ( &CMeshReader::signal_read, this, _1 ) );

  /// @todo future way to handle signatures
  // signal("read").regist_signature( &CMeshReader::signature_read );

  signal("read")->signature->connect( boost::bind(&CMeshReader::read_signature, this, _1) );
}

////////////////////////////////////////////////////////////////////////////////

CMeshReader::~CMeshReader()
{
}

//////////////////////////////////////////////////////////////////////////////

void CMeshReader::signal_read( SignalArgs& node  )
{
  SignalOptions options( node );

  URI path = options.option<URI>("location");

  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the location component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  // get the domain
  Component::Ptr location = access_component_ptr_checked( path );

  std::vector<URI> files = options.array<URI>("files");

  // check protocol for file loading
  boost_foreach(URI file, files)
  {
    if( file.empty() || file.scheme() != URI::Scheme::FILE )
      throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );
  }

  // create a mesh in the domain
  if( !files.empty() )
  {
    CMesh& mesh = location->create_component<CMesh>("Mesh");

    // Get the file paths
    boost_foreach(const URI& file, files)
    {
      read_from_to(file, mesh);
    }
  }
  else
  {
    throw BadValue( FromHere(), "No mesh was read because no files were selected." );
  }
}

//////////////////////////////////////////////////////////////////////////////

CMesh::Ptr CMeshReader::create_mesh_from(const URI& file)
{
  // Create the mesh
  CMesh::Ptr mesh ( allocate_component<CMesh>("mesh") );

  // Call implementation
  read_from_to(file,*mesh);

  // return the mesh
  return mesh;
}

//////////////////////////////////////////////////////////////////////////////

std::map<std::string,CElements::Ptr>
  CMeshReader::create_cells_in_region (CRegion& parent_region, CNodes& nodes,
                                       const std::vector<std::string>& etypes)
{
  std::map<std::string,CElements::Ptr> cells_map;
  boost_foreach(const std::string& etype, etypes)
  {
    ElementType::Ptr element_type = create_component_abstract_type<ElementType>(etype,etype);
    if (element_type->dimensionality() == element_type->dimension())
    {
      CCells& etype_cells = *parent_region.create_component_ptr<CCells>(element_type->shape_name());
      etype_cells.initialize(etype,nodes);
      cells_map[etype] = etype_cells.as_ptr<CElements>();
    }
  }
  return cells_map;
}

////////////////////////////////////////////////////////////////////////////////

std::map<std::string,CElements::Ptr>
  CMeshReader::create_faces_in_region (CRegion& parent_region, CNodes& nodes,
                                       const std::vector<std::string>& etypes)
{
  std::map<std::string,CElements::Ptr> faces_map;
  boost_foreach(const std::string& etype, etypes)
  {
    ElementType::Ptr element_type = create_component_abstract_type<ElementType>(etype,etype);
    if (element_type->dimensionality() == element_type->dimension() - 1)
    {
      CFaces& etype_faces = *parent_region.create_component_ptr<CFaces>(element_type->shape_name());
      etype_faces.initialize(etype,nodes);
      faces_map[etype] = etype_faces.as_ptr<CElements>();
    }
  }
  return faces_map;
}

////////////////////////////////////////////////////////////////////////////////

std::map<std::string,CTable<Uint>::Buffer::Ptr>
  CMeshReader::create_connectivity_buffermap (std::map<std::string,CElements::Ptr>& elems_map)
{
  // Create regions for each element type
  std::map<std::string,CTable<Uint>::Buffer::Ptr> buffermap;
  foreach_container((const std::string& etype)(CElements::Ptr elements), elems_map)
  {
    buffermap[etype] = elements->node_connectivity().create_buffer_ptr();
  }
  return buffermap;
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::remove_empty_element_regions(CRegion& parent_region)
{
  boost_foreach(CElements& region, find_components_recursively<CElements>(parent_region))
  {
    // find the empty regions
    Uint empty_on_this_rank = region.node_connectivity().array().empty();
    Uint empty_on_all_ranks = empty_on_this_rank;

    /// @todo boolean type had to be converted to Uint for it to work
    if (mpi::PE::instance().is_active())
      mpi::all_reduce(mpi::PE::instance(), mpi::logical_and(), &empty_on_this_rank, 1, &empty_on_all_ranks);

    if ( empty_on_all_ranks )
    {
      CElements::Ptr removed = boost::dynamic_pointer_cast<CElements>(region.parent().remove_component(region.name()));
      removed.reset();
    }
  }

  // loop over regions
  boost_foreach(CRegion& region, find_components_recursively<CRegion>(parent_region))
  {
    // find the empty regions
    if ( find_components<CRegion>(region).empty() && find_components<CElements>(region).empty() )
      {
        CRegion::Ptr removed = boost::dynamic_pointer_cast<CRegion>(region.parent().remove_component(region.name()));
        removed.reset();
      }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::read_signature( SignalArgs& node )
{
  SignalOptions options( node );

  std::vector<URI> dummy;
  std::vector<URI::Scheme::Type> schemes(1);

  schemes[0] = URI::Scheme::CPATH;
  options.add("location", URI(), "Component to load mesh into", schemes );
  schemes[0] = URI::Scheme::FILE;
  options.add("files", dummy, " ; " , "Files to read", schemes);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
