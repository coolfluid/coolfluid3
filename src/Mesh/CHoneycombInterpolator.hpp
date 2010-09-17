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
  CHoneycombInterpolator( const CName& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CHoneycombInterpolator"; }
  
  static void defineConfigProperties ( CF::Common::PropertyList& options );

private: // functions

  virtual void construct_internal_storage(const CMesh::Ptr& source, const CMesh::Ptr& target);
  virtual void interpolate_field_from_to(const CField& source, CField& target); 
  void create_honeycomb();
  bool find_comb_idx(const RealVector& coordinate);
  void find_pointcloud(Uint nb_points);
	boost::tuple<CElements::ConstPtr,Uint> find_element(const RealVector& target_coord);
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data
  
  CMesh::Ptr m_source_mesh;
  CMesh::Ptr m_target_mesh;
  CField::Ptr m_source_field;
  CField::Ptr m_target_field;
  
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

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_CHoneycombInterpolator_hpp
