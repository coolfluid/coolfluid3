// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_MeshWriter_hpp
#define cf3_mesh_MeshWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"
#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace common {  class URI;  }
namespace mesh {

  class Mesh;
  class Region;
  class Field;

////////////////////////////////////////////////////////////////////////////////

/// MeshWriter component class
/// This class serves as a component that that will write
/// the mesh to a file
/// @author Willem Deconinck
class Mesh_API MeshWriter : public common::Action {

public: // functions

  /// Contructor
  /// @param name of the component
  MeshWriter ( const std::string& name );

  /// Virtual destructor
  virtual ~MeshWriter();

  /// Get the class name
  static std::string type_name () { return "MeshWriter"; }

  // --------- Signals ---------

  // --------- Direct access ---------

  virtual std::string get_format() = 0;

  virtual std::vector<std::string> get_extensions() = 0;

  virtual void write_from_to(const Mesh& mesh, const common::URI& filepath) = 0;

  virtual void execute();

  /// @deprecated. Configure fields using uri's
  void set_fields(const std::vector<Handle<Field> >& fields);

private: // functions

  void config_fields();  ///< configure fields from URI's
  void config_regions(); ///< configure regions from URI's

private:

  /// Predicate to check if a component directly contains any Entities component
  struct RegionFilter {
    bool enable_interior_faces;
    bool enable_interior_cells;
    bool enable_surfaces;
    bool operator()(const Component& component);
  };

  /// Predicate to check if a Entities component is to be included
  struct EntitiesFilter {
    bool enable_interior_faces;
    bool enable_interior_cells;
    bool enable_surfaces;
    bool operator()(const Component& component);
  };


protected:

  RegionFilter                       m_region_filter;    ///< Filters regions
  EntitiesFilter                     m_entities_filter;  ///< Filters entities
  Handle<Mesh const>                 m_mesh;             ///< Handle to configured mesh
  std::vector<Handle<Field const> >  m_fields;           ///< Handle to configured fields
  std::vector<Handle<Region const> > m_regions;          ///< Handle to configured regions

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MeshWriter_hpp
