#ifndef CF_Tools_Testing_MeshGeneration_hpp
#define CF_Tools_Testing_MeshGeneration_hpp

#include "Mesh/CMesh.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Testing {
  
////////////////////////////////////////////////////////////////////////////////

/// Create a rectangular, 2D, quad-only mesh. No buffer for creation
void create_rectangle(CMesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments);

////////////////////////////////////////////////////////////////////////////////

/// Global fixture that provides unique access to meshes with a certain size
/// Useful for creating large grids outside of profiled fixtures
template<Uint MeshSize>
struct MeshSourceGlobalFixture {
  
  /// Init all meshes at creation time
  MeshSourceGlobalFixture()
  {
    CFinfo << "Created mesh " << grid2().name() << CFendl;
  }
  
  /// Returns a 2D square grid, with side MeshSize
  static const CMesh& grid2()
  {
    static CMesh::Ptr grid2D;
    if(!grid2D) {
      grid2D.reset(new CMesh("grid2D"));
      create_rectangle(*grid2D, 1., 1., MeshSize, MeshSize);
    }
    return *grid2D;
  }
  
};


////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_Testing_MeshGeneration_hpp

