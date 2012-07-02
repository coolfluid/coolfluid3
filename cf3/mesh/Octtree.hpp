// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Octtree_hpp
#define cf3_mesh_Octtree_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "common/BoostArray.hpp"

#include "math/BoundingBox.hpp"

#include "mesh/Elements.hpp"


////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Mesh;

//////////////////////////////////////////////////////////////////////////////

/// This class defines neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API Octtree : public common::Component
{

private: // typedefs

  typedef std::pair<const Elements*,Uint> Point;
  typedef boost::multi_array<std::vector<Entity> ,3> ArrayT;
  typedef std::vector<const Point*> Pointcloud;

public: // functions
  /// constructor
  Octtree( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Octtree"; }

  void create_octtree();

  /// @brief Find which element contains a given coordinate
  /// @param target_coord [in] the given coordinate
  /// @return the elements region, and the local coefficient in this region
  Entity find_element(const RealVector& target_coord);

  /// @brief Find which element contains a given coordinate
  /// @return if element was found
  virtual bool find_element(const RealVector& target_coord, Entity& element);

  /// Given a coordinate, find which box in the octtree it is located in
  /// @param coordinate  [in]  The coordinate to look for
  /// @param octtree_idx [out] location of the box (i,j,k) in which the coordinate sits
  /// @return true if the coordinate is found inside the honeycomb
  bool find_octtree_cell(const RealVector& coordinate, std::vector<Uint>& octtree_idx);

  /// Assemble the elements in and around a given octtree_idx (i,j,k)
  /// It is assumed that first "find_comb_idx(coordinate)" is called
  /// @param octtree_idx [in] the minimum number of points in the point cloud
  /// @param ring        [in] the ring of indexes around octtree_idx to look for elements. Elements inside the ring are not looked for, only the ring itself.
  /// @param unified_elems [out] the elements are pushed back in this vector. Nothing gets erased, it only grows.
  /// @note subsequent calls with increasing value for ring starting from 0, will assemble everything within the last passed ring value.
  void gather_elements_around_idx(const std::vector<Uint>& octtree_idx, const Uint ring, std::vector<Entity>& element_pool);

  void find_cell_ranks( const boost::multi_array<Real,2>& coordinates, std::vector<Uint>& ranks );

  bool is_created() const { return m_octtree.num_elements()!=0; }

  const Uint dimension() { return m_dim; }

private: // data

  ArrayT m_octtree;

  Uint m_dim;
  std::vector<Uint> m_N;
  std::vector<Real> m_D;

  Handle<Mesh> m_mesh;

  std::vector<Uint> m_octtree_idx;

  std::vector<Entity> m_elements_pool;

  math::BoundingBox m_bounding_box;

}; // end Octtree

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_Octtree_hpp
