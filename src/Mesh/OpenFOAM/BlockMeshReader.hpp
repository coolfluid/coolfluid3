// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_OpenFOAM_BlockMeshReader_hpp
#define CF_Mesh_OpenFOAM_BlockMeshReader_hpp

#include "Mesh/CMeshReader.hpp"

namespace CF {
namespace Mesh {
namespace OpenFOAM {

//////////////////////////////////////////////////////////////////////////////

/// This class defines OpenFOAM BlockMesh mesh format reader
/// @author Bart Janssens
class BlockMeshReader : public CMeshReader
{
public:

  /// constructor
  BlockMeshReader(const CName& name);

  /// Gets the Class name
  static std::string type_name() { return "BlockMeshReader"; }
  
  static void defineConfigProperties ( CF::Common::PropertyList& options ) {}

  virtual std::string get_format() { return "blockMeshDict"; }

  virtual std::vector<std::string> get_extensions();
  
  virtual void read_from_to(boost::filesystem::path& path, const CMesh::Ptr& mesh);
  
private:
  static void regist_signals ( BlockMeshReader* self ) {}

}; // end BlockMeshReader


////////////////////////////////////////////////////////////////////////////////

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_OpenFOAM_BlockMeshReader_hpp */
