// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CNodeElementConnectivity_hpp
#define CF_Mesh_CNodeElementConnectivity_hpp

#include "Mesh/CElements.hpp"
#include "Mesh/CUnifiedData.hpp"
#include "Mesh/CDynTable.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
  class CLink;
}
namespace Mesh {
  
  class CNodes;

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between nodes and their adjacent elements
/// and provides a convenient API to access the data
class Mesh_API CNodeElementConnectivity : public Common::Component
{
public:

  typedef boost::shared_ptr<CNodeElementConnectivity> Ptr;
  typedef boost::shared_ptr<CNodeElementConnectivity const> ConstPtr;

  /// Contructor
  /// @param name of the component
  CNodeElementConnectivity ( const std::string& name ) : Component(name)
  {
    m_nodes = create_static_component<Common::CLink>("nodes");
    m_elements = create_static_component<CUnifiedData<CElements> >("elements");
    m_connectivity = create_static_component<CDynTable<Uint> >("connectivity_table");
  }

  /// Virtual destructor
  virtual ~CNodeElementConnectivity() {}

  /// Get the class name
  static std::string type_name () { return "CNodeElementConnectivity"; }

  template <typename ElementsRangeT>
      void setup(CNodes& nodes, const ElementsRangeT& range)
  {
    set_nodes(nodes);
    set_elements(range);
    build_connectivity();
  }

  template<typename ElementsRangeT>
      void set_elements( const ElementsRangeT& range)
  {
    m_elements->set_data(range);
  }
  
  void set_nodes(CNodes& nodes);

  void build_connectivity();

  CDynTable<Uint>::ConstRow elements(const Uint node_index) const;
  
  CUnifiedData<CElements>::data_location_type element_location(const Uint unified_elem_idx);
  
  CUnifiedData<CElements>::const_data_location_type element_location(const Uint unified_elem_idx) const;
  
  const CDynTable<Uint>& connectivity() const { return *m_connectivity; }
  
private: // data

  CUnifiedData<CElements>::Ptr m_elements;
  CDynTable<Uint>::Ptr m_connectivity;
  boost::shared_ptr<Common::CLink> m_nodes;
  
}; // CNodeElementConnectivity

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ConnectivityData_hpp
