// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>

#include "Common/ObjectProvider.hpp"

#include "Mesh/OpenFOAM/BlockData.hpp"
#include "Mesh/OpenFOAM/BlockMeshReader.hpp"
#include "Mesh/OpenFOAM/OpenFOAMAPI.hpp"
#include "Mesh/OpenFOAM/Parser.hpp"
#include "Mesh/OpenFOAM/SimpleCommunicationPattern.hpp"

namespace CF {
namespace Mesh {
namespace OpenFOAM {

using namespace CF::Common;
using namespace CF::Mesh;

////////////////////////////////////////////////////////////////////////////////

CF::Common::ObjectProvider < Mesh::OpenFOAM::BlockMeshReader,
                             Mesh::CMeshReader,
                             Mesh::OpenFOAM::OpenFOAMLib,
                             1 >
aBlockMeshReader_Provider ( "blockMeshDict" );

//////////////////////////////////////////////////////////////////////////////

BlockMeshReader::BlockMeshReader(const Component::CName& name): CMeshReader(name)
{
  BUILD_COMPONENT;
}

std::vector< std::string > BlockMeshReader::get_extensions()
{
  std::vector< std::string > result(1, ".dict");
  return result;
}

void BlockMeshReader::read_from_to(boost::filesystem::path& path, const CMesh::Ptr& mesh)
{
  // if the file is present open it
  boost::filesystem::fstream file;
  if( boost::filesystem::exists(path) )
  {
    CFLog(VERBOSE, "Opening file " <<  path.string() << "\n");
    file.open(path,std::ios_base::in); // exists so open it
  }
  else // doesnt exist so throw exception
  {
     throw boost::filesystem::filesystem_error( path.string() + " does not exist",
                                                boost::system::error_code() );
  }
  
  BlockData block_data;
  parse_blockmesh_dict(file, block_data);
  SimpleCommunicationPattern::IndicesT unused;
  build_mesh(block_data, *mesh, unused);
}

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF
