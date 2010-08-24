//#include <boost/mpl/for_each.hpp>

#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/SF/Types.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/CForAllElements.hpp"
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
    
    CField& volumes = mesh->create_field("volumes");
    volumes.create_data_storage(1, CField::ELEMENT_BASED);

    CFinfo << CFendl << CFendl;
    
    // -------------------------------------------------- Volume Computer
    CFinfo << "Volume Computer, templated" << CFendl;
    CFinfo << "--------------------------" << CFendl;
    // Create volume computer, can be virtual at this level
    COperation::Ptr volume_computer(new CForAllElementsT<CComputeVolumes>("volume_computer")); 

    // Configure the sub-operation, in this case CComputeVolumes
    // This can all be done through ConfigOptions and xml later
    volume_computer->operation().stores(volumes);
    
    // Configure this operation (CForAllElements)
    // This can all be done through ConfigOptions and xml later
    volume_computer->needs(mesh->geometry());
    
    // Execute this operation
    volume_computer->execute(); 
    CFinfo << "... done" << CFendl;
    
    CFinfo << CFendl << CFendl;

    // -------------------------------------------------- Output scalar field
    CFinfo << "Output Volume, templated" << CFendl;
    CFinfo << "------------------------" << CFendl;
    // Create volume computer, concrete in this case
    CForAllElementsT<COutputField> scalarfield_outputer ("scalarfield_outputer");

    // Configure the sub-operation, in this case COutputField
    // This can all be done through ConfigOptions and xml later
    scalarfield_outputer.operation().needs(volumes);
    
    // Configure this operation (CForAllElements)
    // This can all be done through ConfigOptions and xml later
    scalarfield_outputer.needs(mesh->geometry());

    // Execute this operation;
    scalarfield_outputer.execute();
    
    CFinfo << CFendl << CFendl;
        
    
    // --------------------------------------------------- Merged Operation
    CFinfo << "Merge[Volume Computer & Output Volume], templated" << CFendl;
    CFinfo << "-------------------------------------------------" << CFendl;

    CForAllElementsT< COperationMergeT<CComputeVolumes,COutputField> > merged_operator("merged");
    
    // Configuration
    merged_operator.operation().operation1().stores(volumes);
    merged_operator.operation().operation2().needs(volumes);
    merged_operator.needs(mesh->geometry());

    // Execution
    merged_operator.execute();
    
    CFinfo << CFendl << CFendl;
    
    // --------------------------------------------------- Virtual Operation
    CFinfo << "Volume Computer & Output Volume, virtual" << CFendl;
    CFinfo << "----------------------------------------" << CFendl;
    // Create virtual operator, and configure (can be done later through xml)
    CForAllElements::Ptr virtual_operator (new CForAllElements("virtual"), Deleter<CForAllElements>());
      virtual_operator->needs(mesh->geometry());
    
    // Create a virtual operation_1, and configure (can be done later through xml)
    COperation& volume_op = virtual_operator->create_operation("CComputeVolumes");
      volume_op.stores(volumes);
    
    // Create a virtual operation_2, and configure (can be done later through xml)
    COperation& output_op = virtual_operator->create_operation("COutputField");
      output_op.needs(volumes);

    // Execute all
    virtual_operator->execute();
    
    CFinfo << CFendl;
    CFinfo << "virtual_operator datastructure:\n"
           << "-------------------------------\n"
           << virtual_operator->tree() << CFendl;
    
    // Since CForAllElementsT derives from COperation, 
    // loops can be nested both templatized as virtual
    
    
    
    
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


