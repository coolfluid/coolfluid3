#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for ConnectivityData"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/ConnectivityData.hpp"
#include "Mesh/ElementNodes.hpp"

#include "Tools/Testing/Difference.hpp"
#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Tools::Testing;
using namespace CF::Tools::MeshGeneration;

//////////////////////////////////////////////////////////////////////////////

/// Define the global fixture type
typedef MeshSourceGlobalFixture<1000> MeshSource;

/// Fixture providing a simple mesh read from a .neu file. Unprofiled.
struct NeuFixture
{
  /// common setup for each test case
  NeuFixture() : mesh2d(new CMesh  ( "mesh2d" )), mesh3d(new CMesh  ( "mesh3d" ))
  {
    // Read the a .neu mesh as 2D mixed mesh
    boost::shared_ptr<CMeshReader> meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");

    // the file to read from
    boost::filesystem::path fp_in_2d ("quadtriag.neu");
    boost::filesystem::path fp_in_3d ("hextet.neu");

    // Read the mesh
    meshreader->read_from_to(fp_in_2d,mesh2d);
    meshreader->read_from_to(fp_in_3d,mesh3d);
  }

  boost::shared_ptr<CMesh> mesh2d;
  boost::shared_ptr<CMesh> mesh3d;
};

/// Profile and time tests using this fixture
struct ProfiledFixture :
  public ProfiledTestFixture, // NOTE: Inheritance order matters, this way the timing is profiled,
  public TimedTestFixture     //       but the profiling is not timed. Important since especially profile processing takes time.
{
    ProfiledFixture() : grid2D(MeshSource::grid2()) {}
  
  const CMesh& grid2D;
};

/// Print out the connectivity information
void print_connectivity(const Component& root, const bool print_empty = true)
{
  BOOST_FOREACH(const CFaceConnectivity& face_connectivity, recursive_range_typed<CFaceConnectivity>(root))
  {
    CFinfo << "------------------------- Connectivity for " << face_connectivity.get_parent()->path().string() << "/" << face_connectivity.get_parent()->name() << " -------------------------" << CFendl;
    CElements::ConstPtr celements = boost::dynamic_pointer_cast<CElements const>(face_connectivity.get_parent());
    const Uint nb_elements = celements->connectivity_table().array().size();
    const Uint nb_faces = celements->element_type().nb_faces();
    for(Uint elem = 0; elem != nb_elements; ++elem)
    {
      for(Uint face = 0; face != nb_faces; ++face)
      {
        if(face_connectivity.has_adjacent_element(elem, face))
        {
          CFaceConnectivity::ElementReferenceT connected = face_connectivity.adjacent_element(elem, face);
          CFinfo << "Face " << face << " of element " << elem << " is connected to face " << face_connectivity.adjacent_face(elem, face) << " of element " << connected.second << " of CElements " << connected.first->path().string() << "/" << connected.first->name() << CFendl;
        }
        else if(print_empty)
        {
          CFinfo << "Face " << face << " of element " << elem << " has no neighbour" << CFendl;
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( MeshSource )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( ConnectivityDataSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( CreateElementVector, NeuFixture )
{
  CFaceConnectivity::ElementsT celements_vector;
  CFaceConnectivity::IndicesT celements_first_elements;
  create_celements_vector(recursive_range_typed<CElements>(*mesh2d), celements_vector, celements_first_elements);
  
  for(Uint i = 0; i != celements_vector.size(); ++i)
    CFinfo << celements_vector[i]->name() << CFendl;
  
  // Should have 6 element regions
  BOOST_CHECK_EQUAL(celements_vector.size(), static_cast<Uint>(6));
}

BOOST_FIXTURE_TEST_CASE( CreateNodeElementLink, NeuFixture )
{
  CFaceConnectivity::ElementsT celements_vector;
  CFaceConnectivity::IndicesT celements_first_elements;
  create_celements_vector(recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsVolume()), celements_vector, celements_first_elements);
  const CArray& coordinates = recursive_get_named_component_typed<CArray>(*mesh2d, "coordinates");
  CFaceConnectivity::IndicesT node_first_elements;
  CFaceConnectivity::CountsT node_element_counts;
  CFaceConnectivity::IndicesT node_elements;
  create_node_element_connectivity(coordinates.array().size(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements);
  
  for(Uint i = 0; i != node_first_elements.size(); ++i)
  {
    CFinfo << "Node " << i << " is used by elements";
    const Uint elements_begin = node_first_elements[i];
    const Uint elements_end = elements_begin + node_element_counts[i];
    for(Uint j = elements_begin; j != elements_end; ++j)
    {
      CFinfo << " " << node_elements[j];
    }
    CFinfo << CFendl;
  }
  CFinfo << "node_elements = ";
  for(Uint i = 0; i != node_elements.size(); ++i)
    CFinfo << "(" << node_elements[i] << ")";
  CFinfo << CFendl;
  
  CFaceConnectivity::IndicesT reference = boost::assign::list_of(0)(1)(0)(1)(1)(4)(5)(0)(3)(0)(1)(2)(3)(5)(2)(3)(10)(11)(4)(6)(7)(2)(4)(5)(6)(10)(15)(11)(12)(7)(8)(12)(13)(9)(13)(14)(8)(9)(6)(7)(8)(9)(14)(15)(10)(11)(12)(13)(14)(15);
  Accumulator accumulator;
  vector_test(node_elements, reference, accumulator);
  BOOST_CHECK_EQUAL(boost::accumulators::min(accumulator.exact), true);
}

BOOST_FIXTURE_TEST_CASE( CreateFaceConnectivity, NeuFixture )
{
  // Vector of the elements that are concerned
  CFaceConnectivity::ElementsT celements_vector;
  CFaceConnectivity::IndicesT celements_first_elements;
  create_celements_vector(recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsVolume()), celements_vector, celements_first_elements);
  
  // Get the coordinates array
  const CArray& coordinates = recursive_get_named_component_typed<CArray>(*mesh2d, "coordinates");
  
  // Link nodes to the elements
  CFaceConnectivity::IndicesT node_first_elements;
  CFaceConnectivity::CountsT node_element_counts;
  CFaceConnectivity::IndicesT node_elements;
  create_node_element_connectivity(coordinates.array().size(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements);
  
  // Create the face connectivity data for the last CElements
  CFaceConnectivity::BoolsT face_has_neighbour;
  CFaceConnectivity::IndicesT face_element_connectivity;
  create_face_element_connectivity(*celements_vector.back(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements, face_has_neighbour, face_element_connectivity);
  
  // Face-face connectivity
  CFaceConnectivity::IndicesT face_face_connectivity;
  create_face_face_connectivity(*celements_vector.back(), celements_vector, celements_first_elements, face_has_neighbour, face_element_connectivity, face_face_connectivity);
  
  // Output data
  const CElements& elements = *celements_vector.back();
  for(Uint element_idx = 0; element_idx != elements.connectivity_table().array().size(); ++element_idx)
  {
    const Uint nb_faces = elements.element_type().nb_faces();
    for(Uint face_idx = 0; face_idx != nb_faces; ++face_idx)
    {
      const Uint global_face_idx = element_idx*nb_faces + face_idx;
      if(face_has_neighbour[global_face_idx]) // face has a neighbour
      {
        const Uint neighbour_global_idx = face_element_connectivity[global_face_idx];
        const Uint neighbour_celement_idx = std::upper_bound(celements_first_elements.begin(), celements_first_elements.end(), neighbour_global_idx) - 1 - celements_first_elements.begin();
        const Uint neighbour_local_idx = neighbour_global_idx - celements_first_elements[neighbour_celement_idx];
        CFinfo << "Face " << face_idx << " of element " << element_idx << " is connected to face " << face_face_connectivity[global_face_idx] << " of element (" << neighbour_celement_idx << ", " << neighbour_local_idx << ")" << CFendl;
      }
      else // no neighbour for this face
      {
        CFinfo << "Face " << face_idx << " of element " << element_idx << " has no neighbour" << CFendl;
      }
    }
  }
  CFinfo << "face_element_connectivity = ";
  for(Uint i = 0; i != face_element_connectivity.size(); ++i)
    CFinfo << "(" << face_element_connectivity[i] << ")";
  CFinfo << CFendl;
  
  // Check result
  CFaceConnectivity::IndicesT reference = boost::assign::list_of(15)(4)(7)(8)(6)(0)(9)(7)(0)(14)(8)(0)(11)(2)(15)(0)(10)(12)(0)(11)(13)(0)(12)(14)(9)(13)(15)(10)(6)(14);
  Accumulator accumulator;
  vector_test(face_element_connectivity, reference, accumulator);
  BOOST_CHECK_EQUAL(boost::accumulators::min(accumulator.exact), true);
}

// BOOST_FIXTURE_TEST_CASE( ProfileFaceConnectivity, ProfiledFixture )
// {
//   // Vector of the elements that are concerned
//   CFaceConnectivity::ElementsT celements_vector;
//   CFaceConnectivity::IndicesT celements_first_elements;
//   create_celements_vector(recursive_filtered_range_typed<CElements>(grid2D, IsElementsVolume()), celements_vector, celements_first_elements);
//   
//   // Get the coordinates array
//   const CArray& coordinates = recursive_get_named_component_typed<CArray>(grid2D, "coordinates");
//   
//   // Link nodes to the elements
//   CFaceConnectivity::IndicesT node_first_elements;
//   CFaceConnectivity::CountsT node_element_counts;
//   CFaceConnectivity::IndicesT node_elements;
//   create_node_element_connectivity(coordinates.array().size(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements);
//   
//   // Create the face connectivity data
//   CFaceConnectivity::BoolsT face_has_neighbour;
//   CFaceConnectivity::IndicesT face_element_connectivity;
//   create_face_element_connectivity(*celements_vector.back(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements, face_has_neighbour, face_element_connectivity);
//   
//   // Face-face connectivity
//   CFaceConnectivity::IndicesT face_face_connectivity;
//   create_face_face_connectivity(*celements_vector.back(), celements_vector, celements_first_elements, face_has_neighbour, face_element_connectivity, face_face_connectivity);
//   
//   BOOST_CHECK_EQUAL(face_has_neighbour[3], false);
//   BOOST_CHECK_EQUAL(face_has_neighbour[1], true);
//   BOOST_CHECK_EQUAL(face_element_connectivity[1], (Uint) 1);
//   BOOST_CHECK_EQUAL(face_element_connectivity[7], (Uint) 0);
// }

/// Internal connectivity between all volume cells of the mesh
BOOST_FIXTURE_TEST_CASE( CreateVolumeToVolumeConnectivity, NeuFixture )
{
  // Add node connectivity data at the mesh level
  CNodeConnectivity::Ptr node_connectivity = mesh2d->create_component_type<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsVolume()));
  
  // Add face connectivity data for each CElements. Note that we can choose any CElements here, we don't have to do this
  // for the same set as used in node_connectivity
  BOOST_FOREACH(CElements& celements, recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsVolume()))
  {
    celements.create_component_type<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }
  
  print_connectivity(*mesh2d);
}

/// For all surface elements, look up their adjacent volume element
BOOST_FIXTURE_TEST_CASE( CreateSurfaceToVolumeConnectivity, NeuFixture )
{
  // Add node connectivity data at the mesh level
  CNodeConnectivity::Ptr node_connectivity = mesh2d->create_component_type<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsVolume()));
  
  // Add face connectivity data for surface elements
  BOOST_FOREACH(CElements& celements, recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsSurface()))
  {
    celements.create_component_type<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }
  
  print_connectivity(*mesh2d);
  CFinfo << mesh2d->tree() << CFendl;

}

/// For all volume elements, look up their adjacent surface element, if any
BOOST_FIXTURE_TEST_CASE( CreateVolumeToSurfaceConnectivity, NeuFixture )
{
  // Add node connectivity data at the mesh level, for surface elements only
  CNodeConnectivity::Ptr node_connectivity = mesh2d->create_component_type<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsSurface()));
  
  // Add face connectivity data for volume elements
  BOOST_FOREACH(CElements& celements, recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsVolume()))
  {
    celements.create_component_type<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }

  print_connectivity(*mesh2d, false);
  //CFinfo << mesh2d->tree() << CFendl;
}


/// For all surface elements, look up their adjacent volume element
BOOST_FIXTURE_TEST_CASE( CreateSurfaceToVolumeConnectivity3D, NeuFixture )
{
  // Add node connectivity data at the mesh level
  CNodeConnectivity::Ptr node_connectivity = mesh3d->create_component_type<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(recursive_filtered_range_typed<CElements>(*mesh3d, IsElementsVolume()));
  
  
  for(Uint i = 0; i != node_connectivity->node_first_elements().size(); ++i)
  {
    CFinfo << "Node " << i << " is used by elements";
    const Uint elements_begin = node_connectivity->node_first_elements()[i];
    const Uint elements_end = elements_begin + node_connectivity->node_element_counts()[i];
    for(Uint j = elements_begin; j != elements_end; ++j)
    {
      CFinfo << " " << node_connectivity->node_elements()[j];
    }
    CFinfo << CFendl;
  }
  CFinfo << "node_elements = ";
  for(Uint i = 0; i != node_connectivity->node_elements().size(); ++i)
    CFinfo << "(" << node_connectivity->node_elements()[i] << ")";
  CFinfo << CFendl;
  
  CFinfo << mesh3d->tree() << CFendl;

  
  
  // Add face connectivity data for surface elements
  BOOST_FOREACH(CElements& celements, recursive_filtered_range_typed<CElements>(*mesh3d, IsElementsSurface()))
  {
    CFinfo << "surface type = " << celements.full_path().string() << CFendl;
    celements.create_component_type<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }
  
  print_connectivity(*mesh3d);
  CFinfo << mesh3d->tree() << CFendl;
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

