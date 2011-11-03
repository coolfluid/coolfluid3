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

/// @author Tiago Quintino
class Mesh_API LoadMesh : public common::Component {

public: // typedefs

  typedef boost::shared_ptr<LoadMesh> Ptr;
  typedef boost::shared_ptr<LoadMesh const> ConstPtr;

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
  
  /// function load the mesh
  boost::shared_ptr<Mesh> load_mesh(const common::URI& file);

protected: // helper functions

  /// updates the list of avialable readers and regists each one to the extension it supports
  void update_list_of_available_readers();

private: // data

  std::map<std::string,std::vector<mesh::MeshReader::Ptr> > m_extensions_to_readers;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_LoadMesh_hpp