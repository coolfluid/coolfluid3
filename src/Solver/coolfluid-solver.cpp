#include <boost/mpl/for_each.hpp>

#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/SF/Types.hpp"
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
    
    CField& volumes = mesh->create_field("volumes",0,get_component_typed<CRegion>(*mesh));


#define Loop boost::mpl::for_each< SF::Types >
    
    //Loop( ForAllRegions< ComputeVolumes > ( volumes ) );

    CForAllElements<ComputeVolumes> volume_computer ("volume_computer");
    CForAllElements<OutputVolumes> volume_outputer ("volume_outputer");

    volume_computer.loop(volumes);
    volume_outputer.loop(volumes);
    
    CFinfo << "\n\nMerged operation:" <<CFendl;
    CForAllElements<OperationMerge<ComputeVolumes,OutputVolumes> > volume_all_in_1 ("volume_all_in_1");
    volume_all_in_1.loop(volumes);
    //Loop( ForAllRegions< OperationMerge< ComputeVolumes, ComputeVolumes > > ( *mesh ) );
    
    
    CMeshTransformer::Ptr info = create_component_abstract_type<CMeshTransformer>("Info","transformer");
    info->transform(mesh);
    
    //Loop( ForAllVolumes< ComputeVolumes > ( volumes ) );

    //  boost::mpl::for_each< SF::Types >( ForAllVolumes< ComputeVolumes > ( *mesh ) );

    //  boost::mpl::for_each< P1::ElemTypes >( ForAllSurfaces< ComputeNormals > ( *mesh ) );

    //  --------------------------------------------------------------------------

    CFinfo << root->tree() << CFendl;

    //  --------------------------------------------------------------------------

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


