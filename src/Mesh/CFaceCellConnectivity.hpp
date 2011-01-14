// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CFaceCellConnectivity_hpp
#define CF_Mesh_CFaceCellConnectivity_hpp

#include "Mesh/CElements.hpp"
#include "Mesh/CUnifiedData.hpp"
#include "Mesh/CTable.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
  class CLink;
}
namespace Mesh {
  
  class CNodes;
  class CRegion;

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between faces and their adjacent elements
/// and provides a convenient API to access the data
/// @author Andrea Lani
/// @author Willem Deconinck
class Mesh_API CFaceCellConnectivity : public Common::Component
{
public:

  typedef boost::shared_ptr<CFaceCellConnectivity> Ptr;
  typedef boost::shared_ptr<CFaceCellConnectivity const> ConstPtr;

  /// Contructor
  /// @param name of the component
  CFaceCellConnectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~CFaceCellConnectivity() {}

  /// Get the class name
  static std::string type_name () { return "CFaceCellConnectivity"; }

  /// setup the node to element connectivity
  /// This function calls 
  /// - set_nodes(nodes)
  /// - set_elements(elements_range)
  /// - build_connectivity
  /// They could be called seperately if wanted
  /// @post all access functions can be used after setup
  /// @param [in] nodes the nodes component to find connected elements of
  /// @param [in] elements_range the elements range to see if they are connected to the nodes.
  ///                            Can be made using "find_components_recursively<CElements>()" function
  void setup(CRegion& region);

  void setup(CRegion& region1, CRegion& region2);

  /// Build the connectivity table
  /// Build the connectivity table as a CDynTable<Uint>
  /// @pre set_nodes() and set_elements() must have been called
  
  void build_connectivity();

  void build_interface_connectivity();

  /// Find the elements connected to a given node by its index
  /// The return type is CDynTable<Uint>::ConstRow which (or "std::vector<Uint> const&")
  /// @return continuous indices of the elments
  CTable<Uint>::ConstRow elements(const Uint unified_face_idx) const;

  /// Find the element location given a unified element index
  /// @return boost::tuple<CElements::Ptr,Uint>(elem_component,elem_idx)
  CUnifiedData<CElements>::data_location_type element_location(const Uint unified_elem_idx);

  /// Find the element location given a unified element index
  /// @return boost::tuple<CElements::ConstPtr,Uint>(elem_component,elem_idx)  
  CUnifiedData<CElements>::const_data_location_type element_location(const Uint unified_elem_idx) const;
  
  /// const access to the node to element connectivity table in unified indices
  const CTable<Uint>& connectivity() const { return *m_connectivity; }
  
  Uint size() const { return connectivity().size(); }
  
  std::vector<Uint> nodes(const Uint face) const;
  
private: // data

  /// boolean set by configuration to see if 
  /// "CList<Uint> is_bdry" gets stored in CElements
  bool m_store_is_bdry;
  bool m_filter_bdry;

  /// nb_faces
  Uint m_nb_faces;

  /// unified view of the elements
  CUnifiedData<CElements>::Ptr m_elements;

  /// unified view of the elements
  CUnifiedData<CElements>::Ptr m_elements_1;

  /// unified view of the elements
  CUnifiedData<CElements>::Ptr m_elements_2;

  /// Actual connectivity table
  CTable<Uint>::Ptr m_connectivity;
  
  CList<Uint>::Ptr m_face_nb_in_first_elem;
  
  CList<Uint>::Ptr m_is_bdry_face;

}; // CFaceCellConnectivity

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ConnectivityData_hpp
