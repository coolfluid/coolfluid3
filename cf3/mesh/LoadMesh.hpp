// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LoadMesh_hpp
#define cf3_mesh_LoadMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "common/URI.hpp"
#include "mesh/MeshReader.hpp"

#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {
  class Mesh;
////////////////////////////////////////////////////////////////////////////////

/// Helper class to load mesh based on a file extension
/// @author Willem Deconinck
/// @author Tiago Quintino
class Mesh_API LoadMesh : public common::Component {

public: // functions

  /// Contructor
  /// @param name of the component
  LoadMesh ( const std::string& name );

  /// Virtual destructor
  virtual ~LoadMesh();

  /// Get the class name
  static std::string type_name () { return "LoadMesh"; }

  // functions specific to the LoadMesh component

  /// @name SIGNALS
  //@{

  /// signal to load the mesh
  void signal_load_mesh ( common::SignalArgs& node );
  /// signature of signal to load the mesh
  void signature_load_mesh ( common::SignalArgs& node);

  //@} END SIGNALS

  /// Read the file into an existing mesh
  void load_mesh_into(const common::URI& file, Mesh& mesh);

  void load_multiple_files(const std::vector<common::URI>& files, Mesh& mesh);

protected: // helper functions

  /// updates the list of avialable readers and regists each one to the extension it supports
  void update_list_of_available_readers();

private: // data

  std::map<std::string,std::vector<Handle< mesh::MeshReader > > > m_extensions_to_readers;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_LoadMesh_hpp
