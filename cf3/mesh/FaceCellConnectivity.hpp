// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_FaceCellConnectivity_hpp
#define cf3_mesh_FaceCellConnectivity_hpp

//#include "mesh/Elements.hpp"
#include "mesh/UnifiedData.hpp"
#include "common/Table.hpp"
#include "mesh/MeshElements.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  class Link;
}
namespace mesh {

  class FieldGroup;
  
//  class FieldGroup;
  class Region;
  class Cells;

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between faces and their adjacent elements
/// and provides a convenient API to access the data
/// @author Andrea Lani
/// @author Willem Deconinck
class Mesh_API FaceCellConnectivity : public common::Component
{
public:

  typedef boost::shared_ptr<FaceCellConnectivity> Ptr;
  typedef boost::shared_ptr<FaceCellConnectivity const> ConstPtr;

  /// Contructor
  /// @param name of the component
  FaceCellConnectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~FaceCellConnectivity() {}

  /// Get the class name
  static std::string type_name () { return "FaceCellConnectivity"; }

  /// setup the node to element connectivity
  /// This function calls
  /// - set_nodes(nodes)
  /// - set_elements(elements_range)
  /// - build_connectivity
  /// They could be called seperately if wanted
  /// @post all access functions can be used after setup
  /// @param [in] region The region a face cell connectivity will be built from
  void setup(Region& region);

  /// Build the connectivity table
  /// Build the connectivity table as a DynTable<Uint>
  /// @pre set_nodes() and set_elements() must have been called

  void build_connectivity();

  /// const access to the node to element connectivity table in unified indices
  common::Table<Uint>& connectivity() { return *m_connectivity; }

  const common::Table<Uint>& connectivity() const { return *m_connectivity; }

  /// access to see if the face is a bdry face
  common::List<bool>& is_bdry_face() { cf3_assert( is_not_null(m_is_bdry_face) ); return *m_is_bdry_face; }

  const common::List<bool>& is_bdry_face() const { cf3_assert( is_not_null(m_is_bdry_face) ); return *m_is_bdry_face; }

  common::Table<Uint>& face_number() { cf3_assert( is_not_null(m_face_nb_in_elem) ); return *m_face_nb_in_elem; }

  const common::Table<Uint>& face_number() const { cf3_assert( is_not_null(m_face_nb_in_elem) ); return *m_face_nb_in_elem; }

  Uint size() const { return connectivity().size(); }

  std::vector<Uint> face_nodes(const Uint face) const;

  MeshElements& lookup() { cf3_assert_desc("Must build connectivity first", is_not_null(m_mesh_elements)); return *m_mesh_elements; }

  const MeshElements& lookup() const { cf3_assert_desc("Must build connectivity first", is_not_null(m_mesh_elements)); return *m_mesh_elements; }

  std::vector<Component::Ptr> used();

  void add_used (const Component& used_comp);

private: // data

  /// nb_faces
  Uint m_nb_faces;

  /// unified view of the elements
  boost::shared_ptr<common::Group> m_used_components;

  /// Actual connectivity table
  common::Table<Uint>::Ptr m_connectivity;

  common::Table<Uint>::Ptr m_face_nb_in_elem;

  // @todo make a common::List<bool> (some bug prevents using common::List<bool>::Buffer with common::List<bool> )
  common::List<bool>::Ptr m_is_bdry_face;

  MeshElements::Ptr m_mesh_elements;

  bool m_face_building_algorithm;

}; // FaceCellConnectivity

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ConnectivityData_hpp
