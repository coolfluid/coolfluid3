// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_COcttree_hpp
#define CF_Mesh_COcttree_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>

#include "Common/Component.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CUnifiedData.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  class CMesh;

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API COcttree : public Common::Component
{
public: // typedefs

  typedef boost::shared_ptr<COcttree> Ptr;
  typedef boost::shared_ptr<COcttree const> ConstPtr;

private: // typedefs

  typedef std::pair<const CElements*,Uint> Point;
  typedef boost::multi_array<std::vector<Uint> ,3> ArrayT;
  typedef std::vector<const Point*> Pointcloud;

public: // functions
  /// constructor
  COcttree( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "COcttree"; }

  void create_octtree();

  /// Find one single element in which the given coordinate resides.
  /// @param target_coord [in] the given coordinate
  /// @return the elements region, and the local coefficient in this region
  boost::tuple<CElements::ConstPtr,Uint> find_element(const RealVector& target_coord);

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

private: //functions

  /// Create the octtree for fast searching in which element a coordinate can be found
  void create_bounding_box();

  /// Utility function to convert a vector-like type to a RealVector
  template<typename RowT>
  void to_vector(RealVector& result, const RowT& row)
  {
    const Uint row_size = row.size();
    cf_assert(result.size() >= row_size);
    for(Uint i =0; i != row_size; ++i)
      result[i] = row[i];
  }

private: // data

  boost::weak_ptr<CMesh> m_mesh;

  ArrayT m_octtree;

  Uint m_dim;
  enum {MIN=0,MAX=1};
  std::vector< RealVector3, Eigen::aligned_allocator<RealVector3> > m_bounding;
  std::vector<Uint> m_N;
  std::vector<Real> m_D;

  CUnifiedData::Ptr m_elements;

  std::vector<Uint> m_octtree_idx;

}; // end COcttree

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_COcttree_hpp
