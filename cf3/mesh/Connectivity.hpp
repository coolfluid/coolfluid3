// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Connectivity_hpp
#define cf3_mesh_Connectivity_hpp

//#include "mesh/Elements.hpp"
#include "mesh/UnifiedData.hpp"
#include "common/Table.hpp"
#include "common/Group.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  class Link;
  class Group;
}
namespace mesh {

  class Region;
  class Cells;

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between faces and their adjacent elements
/// and provides a convenient API to access the data
/// @author Willem Deconinck
class Mesh_API Connectivity : public common::Table<Uint>
{
public:

  typedef boost::shared_ptr<Connectivity> Ptr;
  typedef boost::shared_ptr<Connectivity const> ConstPtr;

  /// Contructor
  /// @param name of the component
  Connectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~Connectivity() {}

  /// Get the class name
  static std::string type_name () { return "Connectivity"; }

  UnifiedData& lookup();

  const UnifiedData& lookup() const { return *m_lookup; }

  UnifiedData& create_lookup();

  void set_lookup(UnifiedData& lookup);

private: // data

  boost::shared_ptr<UnifiedData> m_lookup;

  boost::shared_ptr<common::Link> m_lookup_link;

}; // Connectivity

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Connectivity_hpp
