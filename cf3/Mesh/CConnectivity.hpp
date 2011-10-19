// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CConnectivity_hpp
#define cf3_Mesh_CConnectivity_hpp

//#include "Mesh/CElements.hpp"
#include "Mesh/CUnifiedData.hpp"
#include "Mesh/CTable.hpp"
#include "common/CGroup.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  class CLink;
  class CGroup;
}
namespace Mesh {

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

  CUnifiedData& lookup();

  const CUnifiedData& lookup() const { return *m_lookup; }

  CUnifiedData& create_lookup();

  void set_lookup(CUnifiedData& lookup);

private: // data

  boost::shared_ptr<CUnifiedData> m_lookup;

  boost::shared_ptr<common::CLink> m_lookup_link;

}; // CConnectivity

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Mesh_Connectivity_hpp
