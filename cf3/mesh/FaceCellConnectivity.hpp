// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_FaceCellConnectivity_hpp
#define cf3_mesh_FaceCellConnectivity_hpp

#include "common/Table.hpp"

#include "mesh/UnifiedData.hpp"
#include "mesh/ElementConnectivity.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  class Link;
}
namespace mesh {

  class SpaceFields;
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
  ElementConnectivity& connectivity() { return *m_connectivity; }

  const ElementConnectivity& connectivity() const { return *m_connectivity; }

  /// access to see if the face is a bdry face
  common::List<bool>& is_bdry_face() { cf3_assert( is_not_null(m_is_bdry_face) ); return *m_is_bdry_face; }

  const common::List<bool>& is_bdry_face() const { cf3_assert( is_not_null(m_is_bdry_face) ); return *m_is_bdry_face; }

  common::Table<Uint>& face_number() { cf3_assert( is_not_null(m_face_nb_in_elem) ); return *m_face_nb_in_elem; }

  const common::Table<Uint>& face_number() const { cf3_assert( is_not_null(m_face_nb_in_elem) ); return *m_face_nb_in_elem; }

  Uint size() const { return connectivity().size(); }

  std::vector<Uint> face_nodes(const Uint face) const;

  std::vector<Component::Ptr> used();

  void add_used (const Component& used_comp);

private: // data

  /// nb_faces
  Uint m_nb_faces;

  /// unified view of the elements
  boost::shared_ptr<common::Group> m_used_components;

  /// Actual connectivity table
  ElementConnectivity::Ptr m_connectivity;

  common::Table<Uint>::Ptr m_face_nb_in_elem;

  // @todo make a common::List<bool> (some bug prevents using common::List<bool>::Buffer with common::List<bool> )
  common::List<bool>::Ptr m_is_bdry_face;

  bool m_face_building_algorithm;

}; // FaceCellConnectivity

////////////////////////////////////////////////////////////////////////////////

struct Face2Cell
{
  Face2Cell() : comp(NULL), idx(0) {}
  Face2Cell(FaceCellConnectivity& component, const Uint index=0) :
    comp( &component ),
    idx( index )
  {
    cf3_assert(idx<comp->size());
  }

  FaceCellConnectivity* comp;
  Uint idx;

  bool is_bdry() const { return comp->is_bdry_face()[idx]; }
  ElementConnectivity::ConstRow cells() const { return comp->connectivity()[idx]; }
  ElementConnectivity::Row cells() { return comp->connectivity()[idx]; }
  common::Table<Uint>::ConstRow face_nb_in_cells() const { return comp->face_number()[idx]; }
  common::Table<Uint>::Row face_nb_in_cells() { return comp->face_number()[idx]; }
  std::vector<Uint> nodes() { return comp->face_nodes(idx); }
  const ElementType& element_type() { return cells()[0].element_type().face_type(face_nb_in_cells()[0]); }

  Face2Cell& operator++()
    { idx++; cf3_assert(idx!=comp->size()); return *this; }
  Face2Cell& operator--()
    { cf3_assert(idx!=0u); idx--; return *this; }
  bool operator==(const Face2Cell& other) const
    { return comp==other.comp && idx==other.idx; }
  bool operator!=(const Face2Cell& other) const
    { return !(*this == other); }

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ConnectivityData_hpp
