//#include <boost/mpl/for_each.hpp>

#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/SF/Types.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/CForAllElements.hpp"
#include "Solver/CForAllNodes.hpp"
// #include "ForAllRegions.hpp"

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
    
    mesh->create_field("volumes",1, CField::ELEMENT_BASED);

    std::vector<URI> regions_to_loop = boost::assign::list_of(URI("cpath://root/mesh/Base"));

    CFinfo << CFendl << CFendl;
    
    // -------------------------------------------------- Volume Computer
    CFinfo << "Volume Computer, templated" << CFendl;
    CFinfo << "--------------------------" << CFendl;
    // Create volume computer, can be virtual at this level
    COperation::Ptr volume_computer =
        root->create_component_type< CForAllElementsT< CComputeVolumes > > ("volume_computer"); 

    ///@todo temporary call bind after construction and before configuration until statically added child-components are allowed.
    volume_computer->bind();

    // Configure the sub-operation, in this case CComputeVolumes
    // Configure the operation
    volume_computer->operation().configure_option(     "Field"   ,   URI("cpath://root/mesh/volumes"   ));
    
    // Configure this operation (CForAllElements)
    volume_computer->configure_option(    "Regions"   , regions_to_loop );
    
    // Execute this operation
    volume_computer->execute(); 
    CFinfo << "... done" << CFendl;
    
    CFinfo << CFendl << CFendl;

    // -------------------------------------------------- Output scalar field
    CFinfo << "Output Volume, templated" << CFendl;
    CFinfo << "------------------------" << CFendl;
    // Create volume writer, concrete in this case
    CForAllElementsT<COutputField>::Ptr scalarfield_outputer = 
        root->create_component_type< CForAllElementsT<COutputField> >("scalarfield_outputer");
    
    ///@todo temporary call bind after construction and before configuration until statically added child-components are allowed.
    scalarfield_outputer->bind();
    
    // Configure the operation
    scalarfield_outputer->operation().configure_option(     "Field"   ,   URI("cpath://root/mesh/volumes"   ));

    // Configure this operation (CForAllElements)
    scalarfield_outputer->configure_option(    "Regions"   , regions_to_loop );

    // Execute this operation;
    scalarfield_outputer->execute();
    
    CFinfo << CFendl << CFendl;
        
    
    // --------------------------------------------------- Merged Operation
    CFinfo << "Merge[Volume Computer & Output Volume], templated" << CFendl;
    CFinfo << "-------------------------------------------------" << CFendl;

    CForAllElementsT< COperationMergeT<CComputeVolumes,COutputField> >::Ptr merged_operator = 
        root->create_component_type<CForAllElementsT< COperationMergeT<CComputeVolumes,COutputField> > >("merged_operator");
    
    ///@todo temporary call bind after construction and before configuration until statically added child-components are allowed.
    merged_operator->bind();
    merged_operator->operation().bind();

    // Configuration
    merged_operator->configure_option(    "Regions"   , regions_to_loop );
    merged_operator->operation().operation1().configure_option(     "Field"   ,   URI("cpath://root/mesh/volumes"   ));
    merged_operator->operation().operation2().configure_option(     "Field"   ,   URI("cpath://root/mesh/volumes"   ));
    

    // Execution
    merged_operator->execute();
    
    CFinfo << CFendl << CFendl;
    
    // --------------------------------------------------- Virtual Operation
    CFinfo << "Volume Computer & Output Volume, virtual" << CFendl;
    CFinfo << "----------------------------------------" << CFendl;
    // Create virtual operator, and configure (can be done through xml)
    CForAllElements::Ptr virtual_operator = 
      root->create_component_type< CForAllElements >("virtual_operator");
    virtual_operator->configure_option(    "Regions"   , regions_to_loop );
    
    // Create a virtual operation_1, and configure (can be done through xml)
    COperation& volume_op = virtual_operator->create_operation("CComputeVolumes");
    volume_op.configure_option(   "Field"   ,   URI("cpath://root/mesh/volumes"   ));
    
    // Create a virtual operation_2, and configure (can be done through xml)
    COperation& output_op = virtual_operator->create_operation("COutputField");
    output_op.configure_option(   "Field"   ,   URI("cpath://root/mesh/volumes"   ));

    // Execute all
    virtual_operator->execute();
    
    CFinfo << CFendl;
    CFinfo << "virtual_operator datastructure:\n"
           << "-------------------------------\n"
           << virtual_operator->tree() << CFendl;
    
    // Since CForAllElementsT derives from COperation, 
    // loops can be nested both templatized as virtual
    
    
    
    mesh->create_field("linear",1, CField::NODE_BASED);

    
    // --------------------------------------------------- Virtual Operation
    CFinfo << "Loop nodes" << CFendl;
    CFinfo << "----------------------------------------" << CFendl;
    CForAllNodes::Ptr node_loop = root->create_component_type<CForAllNodes>("nodes_loop");
    node_loop->configure_option(    "Regions"   , regions_to_loop );
    
    // Create a virtual operation_1, and configure (can be done through xml)
    COperation& setval_op = node_loop->create_operation("CSetValue");
    setval_op.configure_option(   "Field"   ,   URI("cpath://root/mesh/linear"   ));
    CFinfo << "before execution" << CFendl;
    node_loop->execute();
    
    CFinfo << mesh->tree() << CFendl;

    CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
    boost::filesystem::path fp("field.msh");
    meshwriter->write_from_to(mesh,fp);
    
    
//////////////////////////////////////////////////////////////////////////////////////////
// work in progress
///////////////////
//
//    CField& xcoord = mesh->create_field("xcoord",get_component_typed<CRegion>(*mesh));
//    xcoord.create_data_storage(1, CField::ELEMENT_BASED);
//    
//    CForAllElements<OperationMerge<SetX,OutputScalarField> > xcoord_loop ("xcoord");
//    xcoord_loop.loop(xcoord);
//    
//    CField& gradx = mesh->create_field("gradx",get_component_typed<CRegion>(*mesh));
//    gradx.create_data_storage(2, CField::ELEMENT_BASED);
//    
//    CForAllElements<OperationMerge<ComputeGradient,OutputVectorField> > gradient_computer ("gradient_computer");
//    gradient_computer.needs(xcoord);
//    gradient_computer.stores(gradx);
//    gradient_computer.execute();
//
//    
//    CForAllElements<COutputField> output_field ("output_field");
//    output_field.needs(volume);
///////////////////////////////////////////////////////////////////////////////////////////
    
    //Loop( ForAllRegions< OperationMerge< ComputeVolumes, ComputeVolumes > > ( *mesh ) );
    
    CFinfo << CFendl << CFendl << CFendl;
    CMeshTransformer::Ptr info = create_component_abstract_type<CMeshTransformer>("Info","transformer");
    info->transform(mesh);
    
    //Loop( ForAllVolumes< ComputeVolumes > ( volumes ) );

    //  boost::mpl::for_each< SF::Types >( ForAllVolumes< ComputeVolumes > ( *mesh ) );

    //  boost::mpl::for_each< P1::ElemTypes >( ForAllSurfaces< ComputeNormals > ( *mesh ) );

    //  --------------------------------------------------------------------------

    //CFinfo << root->tree() << CFendl;

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


