// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CLinearInterpolator_hpp
#define cf3_Mesh_CLinearInterpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>

#include "Mesh/CInterpolator.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CUnifiedData.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API CLinearInterpolator : public CInterpolator
{
public: // typedefs

  typedef boost::shared_ptr<CLinearInterpolator> Ptr;
  typedef boost::shared_ptr<CLinearInterpolator const> ConstPtr;

private: // typedefs

  typedef std::pair<const CElements*,Uint> Point;
  typedef boost::multi_array<std::vector<Uint> ,3> Honeycomb;
  typedef std::vector<const Point*> Pointcloud;

public: // functions
  /// constructor
  CLinearInterpolator( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CLinearInterpolator"; }

private: // functions

	/// Construct internal storage for fast searching algorithm
	/// @param source [in] the mesh from which interpolation will occur
	virtual void construct_internal_storage(const CMesh& source);

	/// Interpolate from one source field to target field
	/// @param source [in] the source field
	/// @param target [out] the target field
	virtual void interpolate_field_from_to(const Field& source, Field& target);

	/// Create the octtree for fast searching in which element a coordinate can be found
	void create_bounding_box();

	void create_octtree();

  /// Given a coordinate, find which box in the octtree it is located in
  /// @param coordinate [in]  The coordinate to look for
  /// @param point_idx  [out] box location in the octtree (i,j,k)
  /// @return true if the coordinate is found inside the honeycomb
  bool find_point_in_octtree(const RealVector& coordinate, std::vector<Uint>& point_idx);

	/// Find the pointcloud of minimum "nb_points" points
	/// It is assumed that first "find_comb_idx(coordinate)" is called
	/// @param nb_points [in] the minimum number of points in the point cloud
	void find_pointcloud(Uint nb_points);

	/// Find one single element in which the given coordinate resides.
	/// @param target_coord [in] the given coordinate
	/// @return the elements region, and the local coefficient in this region
	boost::tuple<CElements::ConstPtr,Uint> find_element(const RealVector& target_coord);

	/// Pseudo-Laplacian weighted linear interpolation algorithm
	/// @param source_points [in] The coordinates of the points used for interpolation
	/// @param target_point  [in] The coordinate of the target point for interpolation
	/// @param weights [out]  The weights corresponding for each source_point.  Q_t = sum( weight_i * Q_i )
	void pseudo_laplacian_weighted_linear_interpolation(const std::vector<RealVector>& source_points, const RealVector& target_point, std::vector<Real>& weights);


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

  CMesh::ConstPtr m_source_mesh;

  Pointcloud m_pointcloud;
  Honeycomb m_honeycomb;

  Uint m_dim;
  enum {MIN=0,MAX=1};
  std::vector< RealVector3, Eigen::aligned_allocator<RealVector3> > m_bounding;
  std::vector<Uint> m_N;
  std::vector<Real> m_D;
  std::vector<Uint> m_point_idx;

  Uint m_nb_elems;

  Uint m_sufficient_nb_points;

  CUnifiedData::Ptr m_elements;

  std::vector<Uint> m_element_cloud;

}; // end CLinearInterpolator

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Mesh_Neu_CLinearInterpolator_hpp
