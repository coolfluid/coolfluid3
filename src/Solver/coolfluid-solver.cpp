#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/Component.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMesh.hpp"

using namespace CF::Common;
using namespace CF::Mesh;

int main(int argc, char * argv[])
{
  CFinfo << "Welcome to the COOLFLUID K3 solver!\n" << CFflush;

  CoreEnv::instance().initiate(argc, argv);

  
  
  
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
    
  // the file to read from
  boost::filesystem::path inputfile ("quadtriag.neu");
  
  // the mesh to store in
  CRoot::Ptr root = CRoot::create("root");
  CMesh::Ptr mesh = meshreader->create_mesh_from(inputfile);
  root->add_component(mesh);
  
  mesh->print_tree();
  
  
  
  
  
  
  
  CoreEnv::instance().terminate();

  return 0;
}
