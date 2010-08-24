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
#include "Tools/Testing/MeshGeneration.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

/// Define the global fixture type
typedef MeshSourceGlobalFixture<500> MeshSource;

/// Fixture providing a simple mesh read from a .neu file. Unprofiled.
struct NeuFixture
{
  /// common setup for each test case
  NeuFixture() : mesh2d(new CMesh  ( "mesh2d" ))
  {
    // Read the a .neu mesh as 2D mixed mesh
    boost::shared_ptr<CMeshReader> meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");

    // the file to read from
    boost::filesystem::path fp_in ("quadtriag.neu");

    // Read the mesh
    meshreader->read_from_to(fp_in,mesh2d);
  }

  boost::shared_ptr<CMesh> mesh2d;
};

/// Profile and time tests using this fixture
struct ProfiledFixture :
  public ProfiledTestFixture, // NOTE: Inheritance order matters, this way the timing is profiled,
  public TimedTestFixture     //       but the profiling is not timed. Important since especially profile processing takes time.
{
    ProfiledFixture() : grid2D(MeshSource::grid2()) {}
  
  const CMesh& grid2D;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( MeshSource )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( ConnectivityDataSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( CreateElementVector, NeuFixture )
{
  ElementsT celements_vector;
  IndicesT celements_first_elements;
  create_celements_vector(recursive_range_typed<CElements>(*mesh2d), celements_vector, celements_first_elements);
  
  for(Uint i = 0; i != celements_vector.size(); ++i)
    CFinfo << celements_vector[i]->name() << CFendl;
  
  // Should have 6 element regions
  BOOST_CHECK_EQUAL(celements_vector.size(), (Uint) 6);
}

BOOST_FIXTURE_TEST_CASE( CreateNodeElementLink, NeuFixture )
{
  ElementsT celements_vector;
  IndicesT celements_first_elements;
  create_celements_vector(recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsVolume()), celements_vector, celements_first_elements);
  const CArray& coordinates = recursive_get_named_component_typed<CArray>(*mesh2d, "coordinates");
  IndicesT node_first_elements;
  CountsT node_element_counts;
  IndicesT node_elements;
  create_node_element_link(coordinates.array().size(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements);
  
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
  
  IndicesT reference = boost::assign::list_of(0)(1)(0)(1)(1)(4)(5)(0)(3)(0)(1)(2)(3)(5)(2)(3)(10)(11)(4)(6)(7)(2)(4)(5)(6)(10)(15)(11)(12)(7)(8)(12)(13)(9)(13)(14)(8)(9)(6)(7)(8)(9)(14)(15)(10)(11)(12)(13)(14)(15);
  Accumulator accumulator;
  vector_test(node_elements, reference, accumulator);
  BOOST_CHECK_EQUAL(boost::accumulators::min(accumulator.exact), true);
}

BOOST_FIXTURE_TEST_CASE( CreateFaceConnectivity, NeuFixture )
{
  // Vector of the elements that are concerned
  ElementsT celements_vector;
  IndicesT celements_first_elements;
  create_celements_vector(recursive_filtered_range_typed<CElements>(*mesh2d, IsElementsVolume()), celements_vector, celements_first_elements);
  
  // Get the coordinates array
  const CArray& coordinates = recursive_get_named_component_typed<CArray>(*mesh2d, "coordinates");
  
  // Link nodes to the elements
  IndicesT node_first_elements;
  CountsT node_element_counts;
  IndicesT node_elements;
  create_node_element_link(coordinates.array().size(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements);
  
  // Create the face connectivity data
  IndicesT celements_first_faces;
  BoolsT face_has_neighbour;
  IndicesT face_element_connectivity;
  IndicesT face_face_connectivity;
  create_face_connectivity(celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements, celements_first_faces, face_has_neighbour, face_element_connectivity, face_face_connectivity);
  
  // Output data
  for(Uint celement_idx = 0; celement_idx != celements_vector.size(); ++celement_idx)
  {
    const CElements& elements = *celements_vector[celement_idx];
    for(Uint element_idx = 0; element_idx != elements.connectivity_table().table().size(); ++element_idx)
    {
      const Uint nb_faces = elements.element_type().nb_faces();
      for(Uint face_idx = 0; face_idx != nb_faces; ++face_idx)
      {
        const Uint global_face_idx = celements_first_faces[celement_idx] + element_idx*nb_faces + face_idx;
        if(face_has_neighbour[global_face_idx]) // face has a neighbour
        {
          const Uint neighbour_global_idx = face_element_connectivity[global_face_idx];
          const Uint neighbour_celement_idx = std::upper_bound(celements_first_elements.begin(), celements_first_elements.end(), neighbour_global_idx) - 1 - celements_first_elements.begin();
          const Uint neighbour_local_idx = neighbour_global_idx - celements_first_elements[neighbour_celement_idx];
          CFinfo << "Face " << face_idx << " of element (" << celement_idx << ", " << element_idx << ") is connected to face " << face_face_connectivity[global_face_idx] << " of element (" << neighbour_celement_idx << ", " << neighbour_local_idx << ")" << CFendl;
        }
        else // no neighbour for this face
        {
          CFinfo << "Face " << face_idx << " of element (" << celement_idx << ", " << element_idx << ") has no neighbour" << CFendl;
        }
      }
    }
  }
  CFinfo << "face_element_connectivity = ";
  for(Uint i = 0; i != face_element_connectivity.size(); ++i)
    CFinfo << "(" << face_element_connectivity[i] << ")";
  CFinfo << CFendl;
  
  // Check result
  IndicesT reference = boost::assign::list_of(0)(0)(1)(3)(0)(0)(0)(5)(5)(10)(3)(2)(0)(0)(0)(6)(5)(4)(2)(1)(15)(4)(7)(8)(6)(0)(9)(7)(0)(14)(8)(0)(11)(2)(15)(0)(10)(12)(0)(11)(13)(0)(12)(14)(9)(13)(15)(10)(6)(14);
  Accumulator accumulator;
  vector_test(face_element_connectivity, reference, accumulator);
  BOOST_CHECK_EQUAL(boost::accumulators::min(accumulator.exact), true);
}

BOOST_FIXTURE_TEST_CASE( ProfileFaceConnectivity, ProfiledFixture )
{
  // Vector of the elements that are concerned
  ElementsT celements_vector;
  IndicesT celements_first_elements;
  create_celements_vector(recursive_filtered_range_typed<CElements>(grid2D, IsElementsVolume()), celements_vector, celements_first_elements);
  
  // Get the coordinates array
  const CArray& coordinates = recursive_get_named_component_typed<CArray>(grid2D, "coordinates");
  
  // Link nodes to the elements
  IndicesT node_first_elements;
  CountsT node_element_counts;
  IndicesT node_elements;
  create_node_element_link(coordinates.array().size(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements);
  
  // Create the face connectivity data
  IndicesT celements_first_faces;
  BoolsT face_has_neighbour;
  IndicesT face_element_connectivity;
  IndicesT face_face_connectivity;
  create_face_connectivity(celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements, celements_first_faces, face_has_neighbour, face_element_connectivity, face_face_connectivity);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

