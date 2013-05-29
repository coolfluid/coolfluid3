// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CGNS_Writer_hpp
#define cf3_mesh_CGNS_Writer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshWriter.hpp"

#include "mesh/CGNS/LibCGNS.hpp"
#include "mesh/CGNS/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Region;
  class Elements;

namespace CGNS {

//////////////////////////////////////////////////////////////////////////////

/// This class defines CGNS mesh format writer
/// @author Willem Deconinck
class Mesh_CGNS_API Writer : public MeshWriter, public Shared
{

private : // typedefs

  typedef std::vector<Handle<Elements const> > GroupedElements;
  typedef std::map<std::string, GroupedElements > GroupsMapType;

public: // functions

  /// constructor
  Writer( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Writer"; }

  virtual void write();

  virtual std::string get_format() { return "CGNS"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  void write_base(const Mesh& mesh);

  void write_zone(const Region& region, const Mesh& mesh);

  void write_section(const GroupedElements& grouped_elements);

//  void write_boco(const GroupedElements& grouped_elements);

private: // data

  std::string m_fileBasename;

  std::map<const common::Table<Real>*, Uint> m_global_start_idx;

}; // end Writer


////////////////////////////////////////////////////////////////////////////////

} // CGNS
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CGNS_Writer_hpp
