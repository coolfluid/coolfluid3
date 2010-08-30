
#include "Common/Exception.hpp"

#include "Mesh/OpenFOAM/BlockData.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/ConnectivityData.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/ElementNodes.hpp"

#include "Mesh/SF/Hexa3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace OpenFOAM {

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::SF;
  
/// Some helper functions for mesh building
namespace detail {
  
/// looks up node indices based on structured block indices
struct NodeIndices
{
  typedef std::vector<Uint> IndicesT;
  typedef std::vector<Uint> CountsT;
  typedef boost::multi_array<bool, 2> Bools2T;
  
  // Intersection of planes
  enum Bounds { XY = 3, XZ = 4, YZ = 5, XYZ = 6 };
  
  NodeIndices(const CFaceConnectivity& face_connectivity, const BlockData& block_data) : m_face_connectivity(face_connectivity), m_block_data(block_data)
  {
    const Uint nb_blocks = m_block_data.block_subdivisions.size();
    bounded.resize(boost::extents[nb_blocks][7]);
    block_first_nodes.reserve(nb_blocks + 1);
    block_first_nodes.push_back(0);
    for(Uint block = 0; block != nb_blocks; ++block)
    {
      const BlockData::CountsT& segments = m_block_data.block_subdivisions[block];
      const Uint x_segs = segments[XX];
      const Uint y_segs = segments[YY];
      const Uint z_segs = segments[ZZ];
      
      const Uint XPOS = Hexa3DLagrangeP1::XPOS;
      const Uint YPOS = Hexa3DLagrangeP1::YPOS;
      const Uint ZPOS = Hexa3DLagrangeP1::ZPOS;
      
      bounded[block][XX] = m_face_connectivity.adjacent_element(block, XPOS).first->element_type().dimensionality() == DIM_2D;
      bounded[block][YY] = m_face_connectivity.adjacent_element(block, YPOS).first->element_type().dimensionality() == DIM_2D;
      bounded[block][ZZ] = m_face_connectivity.adjacent_element(block, ZPOS).first->element_type().dimensionality() == DIM_2D;
      bounded[block][XY] = bounded[block][XX] && bounded[block][YY];
      bounded[block][XZ] = bounded[block][XX] && bounded[block][ZZ];
      bounded[block][YZ] = bounded[block][YY] && bounded[block][ZZ];
      bounded[block][XYZ] = bounded[block][XX] && bounded[block][YY] && bounded[block][ZZ];
      
      const Uint nb_nodes = x_segs*y_segs*z_segs + 
                            bounded[block][XX]*y_segs*z_segs +
                            bounded[block][YY]*x_segs*z_segs +
                            bounded[block][ZZ]*x_segs*y_segs +
                            bounded[block][XY]*z_segs +
                            bounded[block][XZ]*y_segs +
                            bounded[block][YZ]*x_segs + 
                            bounded[block][XYZ];
                            
      block_first_nodes.push_back(block_first_nodes.back() + nb_nodes);
    }
  }
  
  /// Look up the global node index of node (i, j, k) in block
  /// @param block The block index
  /// @param i Node index in the X direction
  /// @param j Node index in the Y direction
  /// @param k Node index in the Z direction
  Uint operator()(const Uint block, const Uint i, const Uint j, const Uint k)
  {
    cf_assert(block < m_block_data.block_subdivisions.size());
    
    const BlockData::CountsT& segments = m_block_data.block_subdivisions[block];
    const Uint x_segs = segments[XX];
    const Uint y_segs = segments[YY];
    const Uint z_segs = segments[ZZ];
    const Uint nb_internal_nodes = x_segs*y_segs*z_segs;
    
    const Uint XPOS = Hexa3DLagrangeP1::XPOS;
    const Uint YPOS = Hexa3DLagrangeP1::YPOS;
    const Uint ZPOS = Hexa3DLagrangeP1::ZPOS;
    
    cf_assert(i <= x_segs);
    cf_assert(j <= y_segs);
    cf_assert(k <= z_segs);
    
    
    // blocks contain their own nodes, except for XPOS, YPOS and ZPOS planes
    if(i != x_segs && j != y_segs && k != z_segs)
    {
      return block_first_nodes[block] + i + j*x_segs + k*x_segs*y_segs;
    }
    
    // XPOS plane
    if(i == x_segs && j != y_segs && k != z_segs)
    {
      if(!bounded[block][XX])
        return block_first_nodes[m_face_connectivity.adjacent_element(block, XPOS).second] + j*x_segs + k*x_segs*y_segs;
      else
        return block_first_nodes[block] + nb_internal_nodes + j + k*y_segs;
    }
    
    // YPOS plane
    if(i != x_segs && j == y_segs && k != z_segs)
    {
      if(!bounded[block][YY])
        return block_first_nodes[m_face_connectivity.adjacent_element(block, YPOS).second] + i + k*x_segs*y_segs;
      else
        return block_first_nodes[block] + nb_internal_nodes + bounded[block][XX]*y_segs*z_segs + i + k*x_segs;
    }
    
    // ZPOS plane
    if(i != x_segs && j != y_segs && k == z_segs)
    {
      if(!bounded[block][ZZ])
        return block_first_nodes[m_face_connectivity.adjacent_element(block, ZPOS).second] + i + j*x_segs;
      else
        return block_first_nodes[block] + nb_internal_nodes + bounded[block][XX]*y_segs*z_segs + bounded[block][YY]*x_segs*z_segs + i + j*x_segs;
    }
    
    // XPOS and YPOS intersection
    if(i == x_segs && j == y_segs && k != z_segs)
    {
      if(!bounded[block][XY])
      {
        if(!bounded[block][XX])
        {
          const Uint x_adj = m_face_connectivity.adjacent_element(block, XPOS).second;
          return operator()(x_adj, 0, j, k);
        }
        else
        {
          const Uint y_adj = m_face_connectivity.adjacent_element(block, YPOS).second;
          return operator()(y_adj, i, 0, k);
        }
      }
      else
      {
        return block_first_nodes[block] + nb_internal_nodes +
               bounded[block][XX]*y_segs*z_segs +
               bounded[block][YY]*x_segs*z_segs +
               bounded[block][ZZ]*x_segs*y_segs +
               k;
      }
    }
    
    // XPOS and ZPOS intersection
    if(i == x_segs && j != y_segs && k == z_segs)
    {
      if(!bounded[block][XZ])
      {
        if(!bounded[block][XX])
        {
          const Uint x_adj = m_face_connectivity.adjacent_element(block, XPOS).second;
          return operator()(x_adj, 0, j, k);
        }
        else
        {
          const Uint z_adj = m_face_connectivity.adjacent_element(block, ZPOS).second;
          return operator()(z_adj, i, j, 0);
        }
      }
      else
      {
        return block_first_nodes[block] + nb_internal_nodes +
               bounded[block][XX]*y_segs*z_segs +
               bounded[block][YY]*x_segs*z_segs +
               bounded[block][ZZ]*x_segs*y_segs +
               bounded[block][XY]*z_segs +
               j;
      }
    }
    
    // YPOS and ZPOS intersection
    if(i != x_segs && j == y_segs && k == z_segs)
    {
      if(!bounded[block][YZ])
      {
        if(!bounded[block][YY])
        {
          const Uint y_adj = m_face_connectivity.adjacent_element(block, YPOS).second;
          return operator()(y_adj, i, 0, k);
        }
        else
        {
          const Uint z_adj = m_face_connectivity.adjacent_element(block, ZPOS).second;
          return operator()(z_adj, i, j, 0);
        }
      }
      else
      {
        return block_first_nodes[block] + nb_internal_nodes +
               bounded[block][XX]*y_segs*z_segs +
               bounded[block][YY]*x_segs*z_segs +
               bounded[block][ZZ]*x_segs*y_segs +
               bounded[block][XY]*z_segs +
               bounded[block][XZ]*y_segs +
               i;
      }
    }
    
    // XPOS, YPOS and ZPOS intersection
    if(i == x_segs && j == y_segs && k == z_segs)
    {
      if(!bounded[block][XYZ])
      {
        if(!bounded[block][XX])
        {
          const Uint x_adj = m_face_connectivity.adjacent_element(block, XPOS).second;
          return operator()(x_adj, 0, j, k);
        }
        if(!bounded[block][YY])
        {
          const Uint y_adj = m_face_connectivity.adjacent_element(block, YPOS).second;
          return operator()(y_adj, i, 0, k);
        }
        if(!bounded[block][ZZ])
        {
          const Uint z_adj = m_face_connectivity.adjacent_element(block, ZPOS).second;
          return operator()(z_adj, i, j, 0);
        }
      }
      else
      {
        return block_first_nodes[block] + nb_internal_nodes +
              bounded[block][XX]*y_segs*z_segs +
              bounded[block][YY]*x_segs*z_segs +
              bounded[block][ZZ]*x_segs*y_segs +
              bounded[block][XY]*z_segs +
              bounded[block][XZ]*y_segs +
              bounded[block][YZ]*x_segs;
      }
    }
    
    throw ShouldNotBeHere(FromHere(), "Bad node index combination");
  }
  
  /// Index of the first node in the global node array for each block. The last element is actually the total
  /// number of nodes in the mesh
  IndicesT block_first_nodes;
  
  /// For each block, indicate if it is bounded by a boundary patch, in the X, Y, Z, XY, XZ, YZ and XYZ directions
  Bools2T bounded;
  
private:
  const CFaceConnectivity& m_face_connectivity;
  const BlockData& m_block_data;
  
};

/// Create the first step length and expansion rations in each direction
void create_mapped_coords(const Uint segments, BlockData::GradingT::const_iterator gradings, CArray::Array& mapped_coords)
{
  mapped_coords.resize(boost::extents[segments+1][4]);
  for(Uint edge = 0; edge != 4; ++edge)
  {
    Real grading = *(gradings++);
    if(grading != 1.)
    {
      const Real r = pow(grading, 1. / static_cast<Real>(segments - 1)); // expansion ratio
      for(Uint i = 0; i <= segments; ++i)
      {
        mapped_coords[i][edge] = 2. * (1. - pow(r, i)) / (1. - grading*r) - 1.;
      }
    }
    else
    {
      const Real step = 2. / static_cast<Real>(segments);
      for(Uint i = 0; i <= segments; ++i)
      {
        mapped_coords[i][edge] = i*step - 1.;
      }
    }
  }
}
  
} // namespace detail

void build_mesh(const BlockData& block_data, CMesh& mesh)
{
  // This is a "dummy" mesh, in which each element corresponds to a block in the blockMeshDict file.
  // The final mesh will in fact be a refinement of this mesh. Using a CMesh allows us to use the
  // coolfluid connectivity functions to determine inter-block connectivity and the relation to boundary patches.
  CMesh::Ptr block_mesh(new CMesh("block_mesh"));
  
  // root region and coordinates
  CRegion& block_mesh_region = block_mesh->create_region("block_mesh_region");
  CArray& block_coordinates = block_mesh_region.create_coordinates(3);
  
  // Fill the coordinates array
  const Uint nb_nodes = block_data.points.size();
  CArray::Array& coords_array = block_coordinates.array();
  coords_array.resize(boost::extents[nb_nodes][3]);
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
  {
    const BlockData::PointT& point = block_data.points[node_idx];
    coords_array[node_idx][XX] = point[XX];
    coords_array[node_idx][YY] = point[YY];
    coords_array[node_idx][ZZ] = point[ZZ];
  }
  
  // Define the volume cells, i.e. the blocks
  CElements& block_elements = block_mesh_region.create_region("blocks").create_elements("Hexa3DLagrangeP1", block_coordinates);
  CTable::ConnectivityTable& block_connectivity = block_elements.connectivity_table().table();
  const Uint nb_blocks = block_data.block_points.size();
  block_connectivity.resize(boost::extents[nb_blocks][8]);
  for(Uint block_idx = 0; block_idx != nb_blocks; ++block_idx)
  {
    const BlockData::IndicesT& block = block_data.block_points[block_idx];
    std::copy(block.begin(), block.end(), block_connectivity[block_idx].begin());
  }
  
  // Define the surface patches
  const Uint nb_patches = block_data.patch_names.size();
  std::map<std::string, std::string> patch_types; // map patch name to patch type, for later lookup
  for(Uint patch_idx = 0 ; patch_idx != nb_patches; ++patch_idx)
  {
    CElements& patch_elements = block_mesh_region.create_region(block_data.patch_names[patch_idx]).create_elements("Quad3DLagrangeP1", block_coordinates);
    patch_types[block_data.patch_names[patch_idx]] = block_data.patch_types[patch_idx];
    CTable::ConnectivityTable& patch_connectivity = patch_elements.connectivity_table().table();
    const BlockData::IndicesT patch_points = block_data.patch_points[patch_idx];
    const Uint nb_patch_elements = patch_points.size() / 4;
    patch_connectivity.resize(boost::extents[nb_patch_elements][8]);
    for(Uint patch_element_idx = 0; patch_element_idx != nb_patch_elements; ++patch_element_idx)
    {
      std::copy(patch_points.begin() + 4*patch_element_idx, patch_points.begin() + 4*patch_element_idx + 4, patch_connectivity[patch_element_idx].begin());
    }
  }
  
  // Create connectivity data
  CNodeConnectivity::Ptr node_connectivity = block_mesh_region.create_component_type<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(nb_nodes, recursive_range_typed<CElements>(block_mesh_region));
  BOOST_FOREACH(CElements& celements, recursive_range_typed<CElements>(block_mesh_region))
  {
    celements.create_component_type<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);
  }
  
  // Total number of elements
  Uint nb_elements = 0;
  for(Uint block = 0; block != nb_blocks; ++block)
    nb_elements += block_data.block_subdivisions[block][XX] * block_data.block_subdivisions[block][YY] * block_data.block_subdivisions[block][ZZ];
  
  ////////////////////////////////////
  // Create the refined mesh
  ////////////////////////////////////
  
  // Helper data to get the inter-block connectivity right
  detail::NodeIndices nodes(get_component_typed<CFaceConnectivity>(block_elements), block_data);
  
  // Create the node coordinates
  CRegion& root_region = mesh.create_region("root_region");
  CArray& mesh_coords_comp = root_region.create_coordinates(3);
  CArray::Array& mesh_coords = mesh_coords_comp.array();
  mesh_coords.resize(boost::extents[nodes.block_first_nodes.back()][3]);
  
  // Create the volume cells connectivity
  CElements& volume_elements = root_region.create_region("volume").create_elements("Hexa3DLagrangeP1", mesh_coords_comp);
  CTable::ConnectivityTable& volume_connectivity = volume_elements.connectivity_table().table();
  volume_connectivity.resize(boost::extents[nb_elements][8]);
  
  // Fill the volume arrays
  Uint element_idx = 0; // global element index
  for(Uint block = 0; block != nb_blocks; ++block)
  {
    ElementNodeVector block_nodes(8, RealVector(3));
    fill_node_list(block_nodes.begin(), block_coordinates, block_connectivity[block]);
    const BlockData::IndicesT& segments = block_data.block_subdivisions[block];
    const BlockData::GradingT& gradings = block_data.block_gradings[block];
    
    CArray::Array ksi, eta, zta; // Mapped coordinates along each edge
    detail::create_mapped_coords(segments[XX], gradings.begin(), ksi);
    detail::create_mapped_coords(segments[YY], gradings.begin() + 4, eta);
    detail::create_mapped_coords(segments[ZZ], gradings.begin() + 8, zta);
    
    Real w[4][3]; // weights for each edge
    Real w_mag[3]; // Magnitudes of the weights
    for(Uint k = 0; k <= segments[ZZ]; ++k)
    {
      for(Uint j = 0; j <= segments[YY]; ++j)
      {
        for(Uint i = 0; i <= segments[XX]; ++i)
        {
          // Weights are calculating according to the OpenFOAM algorithm
          w[0][KSI] = (1. - ksi[i][0])*(1. - eta[j][0])*(1. - zta[k][0]) + (1. + ksi[i][0])*(1. - eta[j][1])*(1. - zta[k][1]);
          w[1][KSI] = (1. - ksi[i][1])*(1. + eta[j][0])*(1. - zta[k][3]) + (1. + ksi[i][1])*(1. + eta[j][1])*(1. - zta[k][2]);
          w[2][KSI] = (1. - ksi[i][2])*(1. + eta[j][3])*(1. + zta[k][3]) + (1. + ksi[i][2])*(1. + eta[j][2])*(1. + zta[k][2]);
          w[3][KSI] = (1. - ksi[i][3])*(1. - eta[j][3])*(1. + zta[k][0]) + (1. + ksi[i][3])*(1. - eta[j][2])*(1. + zta[k][1]);
          w_mag[KSI] = (w[0][KSI] + w[1][KSI] + w[2][KSI] + w[3][KSI]);
          
          w[0][ETA] = (1. - eta[j][0])*(1. - ksi[i][0])*(1. - zta[k][0]) + (1. + eta[j][0])*(1. - ksi[i][1])*(1. - zta[k][3]);
          w[1][ETA] = (1. - eta[j][1])*(1. + ksi[i][0])*(1. - zta[k][1]) + (1. + eta[j][1])*(1. + ksi[i][1])*(1. - zta[k][2]);
          w[2][ETA] = (1. - eta[j][2])*(1. + ksi[i][3])*(1. + zta[k][1]) + (1. + eta[j][2])*(1. + ksi[i][2])*(1. + zta[k][2]);
          w[3][ETA] = (1. - eta[j][3])*(1. - ksi[i][3])*(1. + zta[k][0]) + (1. + eta[j][3])*(1. - ksi[i][2])*(1. + zta[k][3]);
          w_mag[ETA] = (w[0][ETA] + w[1][ETA] + w[2][ETA] + w[3][ETA]);
          
          w[0][ZTA] = (1. - zta[k][0])*(1. - ksi[i][0])*(1. - eta[j][0]) + (1. + zta[k][0])*(1. - ksi[i][3])*(1. - eta[j][3]);
          w[1][ZTA] = (1. - zta[k][1])*(1. + ksi[i][0])*(1. - eta[j][1]) + (1. + zta[k][1])*(1. + ksi[i][3])*(1. - eta[j][2]);
          w[2][ZTA] = (1. - zta[k][2])*(1. + ksi[i][1])*(1. + eta[j][1]) + (1. + zta[k][2])*(1. + ksi[i][2])*(1. + eta[j][2]);
          w[3][ZTA] = (1. - zta[k][3])*(1. - ksi[i][1])*(1. + eta[j][0]) + (1. + zta[k][3])*(1. - ksi[i][2])*(1. + eta[j][3]);
          w_mag[ZTA] = (w[0][ZTA] + w[1][ZTA] + w[2][ZTA] + w[3][ZTA]);
          
          // Get the mapped coordinates of the node to add
          RealVector mapped_coords(3);
          mapped_coords[KSI] = (w[0][KSI]*ksi[i][0] + w[1][KSI]*ksi[i][1] + w[2][KSI]*ksi[i][2] + w[3][KSI]*ksi[i][3]) / w_mag[KSI];
          mapped_coords[ETA] = (w[0][ETA]*eta[j][0] + w[1][ETA]*eta[j][1] + w[2][ETA]*eta[j][2] + w[3][ETA]*eta[j][3]) / w_mag[ETA];
          mapped_coords[ZTA] = (w[0][ZTA]*zta[k][0] + w[1][ZTA]*zta[k][1] + w[2][ZTA]*zta[k][2] + w[3][ZTA]*zta[k][3]) / w_mag[ZTA];
          
          // Transform to real coordinates
          RealVector coords(0., 3);
          eval<Hexa3DLagrangeP1>(mapped_coords, block_nodes, coords);
          
          // Store the result
          const Uint node_idx = nodes(block, i, j, k);
          mesh_coords[node_idx][XX] = coords[XX];
          mesh_coords[node_idx][YY] = coords[YY];
          mesh_coords[node_idx][ZZ] = coords[ZZ];
        }
      }
    }
    
    // Fill the volume connectivity table
    for(Uint k = 0; k != segments[ZZ]; ++k)
    {
      for(Uint j = 0; j != segments[YY]; ++j)
      {
        for(Uint i = 0; i != segments[XX]; ++i)
        {
          CTable::Row element_connectivity = volume_connectivity[element_idx++];
          element_connectivity[0] = nodes(block, i  , j  , k  );
          element_connectivity[1] = nodes(block, i+1, j  , k  );
          element_connectivity[2] = nodes(block, i+1, j+1, k  );
          element_connectivity[3] = nodes(block, i  , j+1, k  );
          element_connectivity[4] = nodes(block, i  , j  , k+1);
          element_connectivity[5] = nodes(block, i+1, j  , k+1);
          element_connectivity[6] = nodes(block, i+1, j+1, k+1);
          element_connectivity[7] = nodes(block, i  , j+1, k+1);
        }
      }
    }
  }
  
  // Create the boundary elements
  BOOST_FOREACH(CElements& patch_block, recursive_filtered_range_typed<CElements>(block_mesh_region, IsElementsSurface()))
  {
    CFaceConnectivity& adjacency_data = get_component_typed<CFaceConnectivity>(patch_block);
    // Create the volume cells connectivity
    CElements& patch_elements = root_region.create_region(patch_block.get_parent()->name()).create_elements("Quad3DLagrangeP1", mesh_coords_comp);
    CTable::ConnectivityTable& patch_connectivity = patch_elements.connectivity_table().table();
    
    const Uint nb_patches = patch_block.connectivity_table().table().size();
    for(Uint patch_idx = 0; patch_idx != nb_patches; ++patch_idx)
    {
      const Uint adjacent_face = adjacency_data.adjacent_face(patch_idx, 0);
      const CFaceConnectivity::ElementReferenceT block = adjacency_data.adjacent_element(patch_idx, 0);
      const BlockData::CountsT& segments = block_data.block_subdivisions[block.second];
      if(adjacent_face == Hexa3DLagrangeP1::XNEG || adjacent_face == Hexa3DLagrangeP1::XPOS)
      {
        const Uint patch_begin = patch_connectivity.size();
        const Uint patch_end = patch_begin + segments[YY]*segments[ZZ];
        patch_connectivity.resize(boost::extents[patch_end][4]);
        const Uint i = adjacent_face == Hexa3DLagrangeP1::XNEG ? 0 : segments[XX];
        for(Uint k = 0; k != segments[ZZ]; ++k)
        {
          for(Uint j = 0; j != segments[YY]; ++j)
          {
            CTable::Row element_connectivity = patch_connectivity[patch_begin + k*segments[YY] + j];
            element_connectivity[0] = nodes(block.second, i  , j  , k  );
            element_connectivity[adjacent_face == Hexa3DLagrangeP1::XNEG ? 1 : 3] = nodes(block.second, i  , j  , k+1);
            element_connectivity[2] = nodes(block.second, i  , j+1, k+1);
            element_connectivity[adjacent_face == Hexa3DLagrangeP1::XNEG ? 3 : 1] = nodes(block.second, i  , j+1, k  );
          }
        }
      }
      else if(adjacent_face == Hexa3DLagrangeP1::YNEG || adjacent_face == Hexa3DLagrangeP1::YPOS)
      {
        const Uint patch_begin = patch_connectivity.size();
        const Uint patch_end = patch_begin + segments[XX]*segments[ZZ];
        patch_connectivity.resize(boost::extents[patch_end][4]);
        const Uint j = adjacent_face == Hexa3DLagrangeP1::YNEG ? 0 : segments[YY];
        for(Uint k = 0; k != segments[ZZ]; ++k)
        {
          for(Uint i = 0; i != segments[XX]; ++i)
          {
            CTable::Row element_connectivity = patch_connectivity[patch_begin + k*segments[XX] + i];
            element_connectivity[0] = nodes(block.second, i  , j  , k  );
            element_connectivity[adjacent_face == Hexa3DLagrangeP1::YNEG ? 3 : 1] = nodes(block.second, i  , j  , k+1);
            element_connectivity[2] = nodes(block.second, i+1, j  , k+1);
            element_connectivity[adjacent_face == Hexa3DLagrangeP1::YNEG ? 1 : 3] = nodes(block.second, i+1, j  , k);
          }
        }
      }
      else if(adjacent_face == Hexa3DLagrangeP1::ZNEG || adjacent_face == Hexa3DLagrangeP1::ZPOS)
      {
        const Uint patch_begin = patch_connectivity.size();
        const Uint patch_end = patch_begin + segments[XX]*segments[YY];
        patch_connectivity.resize(boost::extents[patch_end][4]);
        const Uint k = adjacent_face == Hexa3DLagrangeP1::ZNEG ? 0 : segments[ZZ];
        for(Uint j = 0; j != segments[YY]; ++j)
        {
          for(Uint i = 0; i != segments[XX]; ++i)
          {
            CTable::Row element_connectivity = patch_connectivity[patch_begin + j*segments[XX] + i];
            element_connectivity[0] = nodes(block.second, i  , j  , k  );
            element_connectivity[adjacent_face == Hexa3DLagrangeP1::ZNEG ? 1 : 3] = nodes(block.second, i  , j+1, k  );
            element_connectivity[2] = nodes(block.second, i+1, j+1, k  );
            element_connectivity[adjacent_face == Hexa3DLagrangeP1::ZNEG ? 3 : 1] = nodes(block.second, i+1, j  , k  );
          }
        }
      }
      else
      {
        throw ShouldNotBeHere(FromHere(), "Invalid patch data");
      }
    }
  }
}

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF
