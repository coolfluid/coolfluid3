// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CHoneycombInterpolator_hpp
#define CF_Mesh_CHoneycombInterpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>

#include "Mesh/CInterpolator.hpp"
#include "Mesh/CElements.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API CHoneycombInterpolator : public CInterpolator
{
public: // typedefs

  typedef boost::shared_ptr<CHoneycombInterpolator> Ptr;
  typedef boost::shared_ptr<CHoneycombInterpolator const> ConstPtr;

private: // typedefs

  typedef std::pair<const CElements*,Uint> Point;
  typedef boost::multi_array<std::vector<Point> ,3> Honeycomb;
  typedef std::vector<const Point*> Pointcloud;
  
public: // functions  
  /// constructor
  CHoneycombInterpolator( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CHoneycombInterpolator"; }
  
  static void define_config_properties ();

private: // functions

	/// Construct internal storage for fast searching algorithm
	/// @param source [in] the mesh from which interpolation will occur
  virtual void construct_internal_storage(const CMesh::Ptr& source);
	
	/// Interpolate from one source field to target field
	/// @param source [in] the source field
	/// @param target [out] the target field
  virtual void interpolate_field_from_to(const CField& source, CField& target); 

  /// Create the honeycomb for fast searching in which element a coordinate can be found
	void create_honeycomb();
	
	/// Given a coordinate, find which box in the honeycomb it is located in
  /// @param coordinate [in] The coordinate to look for
	/// @return if the coordinate is found inside the honeycomb
	bool find_comb_idx(const RealVector& coordinate);
	
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
	/// @param target_points [in] The coordinate of the target point for interpolation
	/// @param weights [out]  The weights corresponding for each source_point.  Q_t = sum( weight_i * Q_i )
	void pseudo_laplacian_weighted_linear_interpolation(const std::vector<RealVector>& source_points, const RealVector& target_point, std::vector<Real>& weights);

  /// regists all the signals declared in this class
  virtual void define_signals () {}

private: // data
  
  CMesh::Ptr m_source_mesh;
  
  Pointcloud m_pointcloud;
  Honeycomb m_honeycomb;
  
  Uint m_dim;
  std::vector<RealVector> m_ranges;
  std::vector<Uint> m_N;
  std::vector<Real> m_D;
  std::vector<Uint> m_comb_idx;

  Uint m_nb_elems;
  
  Uint m_sufficient_nb_points;

}; // end CHoneycombInterpolator

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_CHoneycombInterpolator_hpp
