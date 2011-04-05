// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CConnectivity_hpp
#define CF_Mesh_CConnectivity_hpp

//#include "Mesh/CElements.hpp"
#include "Mesh/CUnifiedData.hpp"
#include "Mesh/CTable.hpp"
#include "Common/CGroup.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
  class CLink;
  class CGroup;
}
namespace Mesh {
  
  class CNodes;
  class CRegion;
  class CCells;

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between faces and their adjacent elements
/// and provides a convenient API to access the data
/// @author Willem Deconinck
class Mesh_API CConnectivity : public Mesh::CTable<Uint>
{
public:

  typedef boost::shared_ptr<CConnectivity> Ptr;
  typedef boost::shared_ptr<CConnectivity const> ConstPtr;

  /// Contructor
  /// @param name of the component
  CConnectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~CConnectivity() {}

  /// Get the class name
  static std::string type_name () { return "CConnectivity"; }

  void add(Component& new_connected);

  template <typename T>
  std::vector<boost::shared_ptr<T> > connected()
  {
    std::vector<boost::shared_ptr<T> > vec;
    Common::ComponentIterator<Common::Component> it = m_connected->begin();
    Common::ComponentIterator<Common::Component> it_end = m_connected->end();
    for (; it != it_end; ++it )
      vec.push_back(it->follow()->as_ptr_checked<T>());
    return vec;
  }

private: // data

  boost::shared_ptr<Common::CGroup> m_connected;

  boost::shared_ptr<CUnifiedData> m_lookup;
}; // CConnectivity

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Connectivity_hpp
