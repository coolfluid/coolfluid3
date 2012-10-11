// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for ConnectivityData"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/FindComponents.hpp"


#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "common/Table.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/ConnectivityData.hpp"

#include "Tools/Testing/Difference.hpp"
#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;
using namespace cf3::Tools::Testing;
using namespace cf3::Tools::MeshGeneration;

//////////////////////////////////////////////////////////////////////////////

/// Define the global fixture type
typedef MeshSourceGlobalFixture<1000> MeshSource;

/// Fixture providing a simple mesh read from a .neu file. Unprofiled.
struct neuFixture
{
  /// common setup for each test case
  neuFixture()
  {
    mesh2d = Core::instance().root().create_component<Mesh>("mesh2d");
    mesh3d = Core::instance().root().create_component<Mesh>("mesh3d");

    // Read the a .neu mesh as 2D mixed mesh
    boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

    // Read the mesh
    meshreader->read_mesh_into("../../resources/quadtriag.neu",*mesh2d);
    meshreader->read_mesh_into("../../resources/hextet.neu",*mesh3d);
  }

  Handle<Mesh> mesh2d;
  Handle<Mesh> mesh3d;
};

/// Profile and time tests using this fixture
struct ProfiledFixture :
  public ProfiledTestFixture, // NOTE: Inheritance order matters, this way the timing is profiled,
  public TimedTestFixture     //       but the profiling is not timed. Important since especially profile processing takes time.
{
    ProfiledFixture() : grid2D(MeshSource::grid2()) {}

  const Mesh& grid2D;
};

/// Print out the connectivity information
void print_connectivity(const Component& root, const bool print_empty = true)
{
  BOOST_FOREACH(const CFaceConnectivity& face_connectivity, find_components_recursively<CFaceConnectivity>(root))
  {
    CFinfo << "------------------------- Connectivity for " << face_connectivity.parent()->uri().base_path().string() << "/" << face_connectivity.parent()->name() << " -------------------------" << CFendl;
    Handle<Elements const> celements(face_connectivity.parent());
    const Uint nb_elements = celements->geometry_space().connectivity().array().size();
    const Uint nb_faces = celements->element_type().nb_faces();
    for(Uint elem = 0; elem != nb_elements; ++elem)
    {
      for(Uint face = 0; face != nb_faces; ++face)
      {
        if(face_connectivity.has_adjacent_element(elem, face))
        {
          CFaceConnectivity::ElementReferenceT connected = face_connectivity.adjacent_element(elem, face);
          CFinfo << "Face " << face << " of element " << elem << " is connected to face " << face_connectivity.adjacent_face(elem, face) << " of element " << connected.second << " of Elements " << connected.first->uri().base_path().string() << "/" << connected.first->name() << CFendl;
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

BOOST_FIXTURE_TEST_CASE( CreateElementVector, neuFixture )
{
  CFaceConnectivity::ElementsT celements_vector;
  CFaceConnectivity::IndicesT celements_first_elements;
  create_celements_vector(find_components_recursively<Elements>(*mesh2d), celements_vector, celements_first_elements);

  for(Uint i = 0; i != celements_vector.size(); ++i)
    CFinfo << celements_vector[i]->name() << CFendl;

  // Should have 6 element regions
  BOOST_CHECK_EQUAL(celements_vector.size(), static_cast<Uint>(6));
}

BOOST_FIXTURE_TEST_CASE( CreateNodeElementLink, neuFixture )
{
  CFaceConnectivity::ElementsT celements_vector;
  CFaceConnectivity::IndicesT celements_first_elements;
  create_celements_vector(find_components_recursively_with_filter<Elements>(*mesh2d, IsElementsVolume()), celements_vector, celements_first_elements);
  const Table<Real>& coordinates = find_component_recursively_with_name<Table<Real> >(*mesh2d, mesh::Tags::coordinates());
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

  CFaceConnectivity::IndicesT reference = boost::assign::list_of(10)(11)(10)(11)(11)(14)(15)(10)(13)(10)(11)(12)(13)(15)(4)(5)(12)(13)(0)(1)(14)(0)(4)(9)(12)(14)(15)(5)(6)(1)(2)(6)(7)(3)(7)(8)(2)(3)(0)(1)(2)(3)(8)(9)(4)(5)(6)(7)(8)(9);
  Accumulator accumulator;
  vector_test(node_elements, reference, accumulator);
  BOOST_CHECK_EQUAL(boost::accumulators::min(accumulator.exact), true);
}

BOOST_FIXTURE_TEST_CASE( CreateFaceConnectivity, neuFixture )
{
  // Vector of the elements that are concerned
  CFaceConnectivity::ElementsT celements_vector;
  CFaceConnectivity::IndicesT celements_first_elements;
  create_celements_vector(find_components_recursively_with_filter<Elements>(*mesh2d, IsElementsVolume()), celements_vector, celements_first_elements);

  // Get the coordinates array
  const Table<Real>& coordinates = find_component_recursively_with_name<Table<Real> >(*mesh2d, mesh::Tags::coordinates());

  // Link nodes to the elements
  CFaceConnectivity::IndicesT node_first_elements;
  CFaceConnectivity::CountsT node_element_counts;
  CFaceConnectivity::IndicesT node_elements;
  create_node_element_connectivity(coordinates.array().size(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements);

  // Create the face connectivity data for the last Elements
  CFaceConnectivity::BoolsT face_has_neighbour;
  CFaceConnectivity::IndicesT face_element_connectivity;
  CFaceConnectivity::IndicesT face_face_connectivity;
  create_face_element_connectivity(*celements_vector.back(), celements_vector, celements_first_elements, node_first_elements, node_element_counts, node_elements, face_has_neighbour, face_element_connectivity, face_face_connectivity);

  // Output data
  const Elements& elements = *celements_vector.back();
  for(Uint element_idx = 0; element_idx != elements.geometry_space().connectivity().array().size(); ++element_idx)
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
  CFaceConnectivity::IndicesT reference = boost::assign::list_of(15)(4)(13)(12)(0)(10)(0)(0)(15)(14)(12)(11);
  Accumulator accumulator;
  vector_test(face_element_connectivity, reference, accumulator);
  BOOST_CHECK_EQUAL(boost::accumulators::min(accumulator.exact), true);
}

// BOOST_FIXTURE_TEST_CASE( ProfileFaceConnectivity, ProfiledFixture )
// {
//   // Vector of the elements that are concerned
//   CFaceConnectivity::ElementsT celements_vector;
//   CFaceConnectivity::IndicesT celements_first_elements;
//   create_celements_vector(find_components_recursively_with_filter<Elements>(grid2D, IsElementsVolume()), celements_vector, celements_first_elements);
//
//   // Get the coordinates array
//   const Table<Real>& coordinates = find_component_recursively_with_name<Table<Real> >(grid2D, mesh::Tags::coordinates());
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
BOOST_FIXTURE_TEST_CASE( CreateVolumeToVolumeConnectivity, neuFixture )
{
  // Add node connectivity data at the mesh level
  Handle<CNodeConnectivity> node_connectivity = mesh2d->create_component<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(find_components_recursively_with_filter<Elements>(*mesh2d, IsElementsVolume()));

  // Add face connectivity data for each Elements. Note that we can choose any Elements here, we don't have to do this
  // for the same set as used in node_connectivity
  BOOST_FOREACH(Elements& celements, find_components_recursively_with_filter<Elements>(*mesh2d, IsElementsVolume()))
  {
    celements.create_component<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }

  print_connectivity(*mesh2d);
}

/// For all surface elements, look up their adjacent volume element
BOOST_FIXTURE_TEST_CASE( CreateSurfaceToVolumeConnectivity, neuFixture )
{
  // Add node connectivity data at the mesh level
  Handle<CNodeConnectivity> node_connectivity = mesh2d->create_component<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(find_components_recursively_with_filter<Elements>(*mesh2d, IsElementsVolume()));

  // Add face connectivity data for surface elements
  BOOST_FOREACH(Elements& celements, find_components_recursively_with_filter<Elements>(*mesh2d, IsElementsSurface()))
  {
    celements.create_component<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }

  print_connectivity(*mesh2d);
  CFinfo << mesh2d->tree() << CFendl;

}

/// For all volume elements, look up their adjacent surface element, if any
BOOST_FIXTURE_TEST_CASE( CreateVolumeToSurfaceConnectivity, neuFixture )
{
  // Add node connectivity data at the mesh level, for surface elements only
  Handle<CNodeConnectivity> node_connectivity = mesh2d->create_component<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(find_components_recursively_with_filter<Elements>(*mesh2d, IsElementsSurface()));

  // Add face connectivity data for volume elements
  BOOST_FOREACH(Elements& celements, find_components_recursively_with_filter<Elements>(*mesh2d, IsElementsVolume()))
  {
    celements.create_component<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }

  print_connectivity(*mesh2d, false);
  //CFinfo << mesh2d->tree() << CFendl;
}


/// For all surface elements, look up their adjacent volume element
BOOST_FIXTURE_TEST_CASE( CreateSurfaceToVolumeConnectivity3D, neuFixture )
{
  // Add node connectivity data at the mesh level
  Handle<CNodeConnectivity> node_connectivity = mesh3d->create_component<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(find_components_recursively_with_filter<Elements>(*mesh3d, IsElementsVolume()));


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
  BOOST_FOREACH(Elements& celements, find_components_recursively_with_filter<Elements>(*mesh3d, IsElementsSurface()))
  {
    CFinfo << "surface type = " << celements.uri().string() << CFendl;
    celements.create_component<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }

  print_connectivity(*mesh3d);
  CFinfo << mesh3d->tree() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

