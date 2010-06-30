#include <boost/mpl/for_each.hpp>

#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/P1/ElemTypes.hpp"
#include "Mesh/CTable.hpp"

#include "ComputeVolumes.hpp"
#include "ForAllRegions.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

/////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  CFinfo << "COOLFLUID K3 Solver" << CFendl;

  CoreEnv::instance().initiate(argc, argv);
  
  try
  {

    // read the mesh
    CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
    
    // the file to read from
    boost::filesystem::path inputfile ("quadtriag.neu");

    // the mesh to store in
    CRoot::Ptr root = CRoot::create("root");
    CMesh::Ptr mesh = meshreader->create_mesh_from(inputfile);
    root->add_component(mesh);
    
    
    boost::mpl::for_each< P1::ElemTypes >( ForAllRegions< ComputeVolumes > ( *mesh ) );

    //  boost::mpl::for_each< P1::ElemTypes >( ForAllVolumes<  OperationMerge< ComputeVolumes, ComputeWallDistance > > ( *mesh ) );

    //  boost::mpl::for_each< P1::ElemTypes >( ForAllVolumes< ComputeWallDistance > ( *mesh ) );

    //  boost::mpl::for_each< P1::ElemTypes >( ForAllSurfaces< ComputeNormals > ( *mesh ) );

    //--------------------------------------------------------------------------

    CFinfo << root->tree() << CFendl;

    //--------------------------------------------------------------------------

  }
  catch ( std::exception& ex )
  {
    CFerror << "Unhandled exception: " << ex.what() << CFendl;
  }
  catch ( ... )
  {
    CFerror << "Detected unknown exception" << CFendl;
  }

  CoreEnv::instance().terminate();

  return 0;
}


///////////////////////////////////////////////////////////////////////////////


