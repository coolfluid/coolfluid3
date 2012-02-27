// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Octtree_hpp
#define cf3_mesh_Octtree_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>

#include "common/Component.hpp"
#include "mesh/BoundingBox.hpp"
#include "mesh/Elements.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Mesh;
  class UnifiedData;

//////////////////////////////////////////////////////////////////////////////

/// This class defines neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API Octtree : public common::Component
{
public: // typedefs




private: // typedefs

  typedef std::pair<const Elements*,Uint> Point;
  typedef boost::multi_array<std::vector<Uint> ,3> ArrayT;
  typedef std::vector<const Point*> Pointcloud;

public: // functions
  /// constructor
  Octtree( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Octtree"; }

  void create_octtree();

  /// Find one single element in which the given coordinate resides.
  /// @param target_coord [in] the given coordinate
  /// @return the elements region, and the local coefficient in this region
  boost::tuple<Handle< Elements >,Uint> find_element(const RealVector& target_coord);


  bool find_element(const RealVector& target_coord, Handle< Elements >& element_component, Uint& element_idx);

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
  void gather_elements_around_idx(const std::vector<Uint>& octtree_idx, const Uint ring, std::vector<Uint>& unified_elems);

  void find_cell_ranks( const boost::multi_array<Real,2>& coordinates, std::vector<Uint>& ranks );

private: //functions

  /// Create the octtree for fast searching in which element a coordinate can be found
  void create_bounding_box();

  /// Utility function to convert a vector-like type to a RealVector
  template<typename RowT>
  void to_vector(RealVector& result, const RowT& row)
  {
    const Uint row_size = row.size();
    cf3_assert(result.size() >= row_size);
    for(Uint i =0; i != row_size; ++i)
      result[i] = row[i];
  }



private: // data

  ArrayT m_octtree;

  Uint m_dim;
  Handle<BoundingBox> m_bounding_box;
  std::vector<Uint> m_N;
  std::vector<Real> m_D;

  Handle<UnifiedData> m_elements;
  Handle<Mesh> m_mesh;

  std::vector<Uint> m_octtree_idx;

}; // end Octtree

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_Octtree_hpp
