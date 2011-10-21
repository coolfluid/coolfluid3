// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_WriteMesh_hpp
#define cf3_mesh_WriteMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"
#include "common/URI.hpp"
#include "mesh/CMeshWriter.hpp"

#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {
  class CMesh;
////////////////////////////////////////////////////////////////////////////////

/// @author Tiago Quintino
class Mesh_API WriteMesh : public common::Action {

public: // typedefs

  typedef boost::shared_ptr<WriteMesh> Ptr;
  typedef boost::shared_ptr<WriteMesh const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  WriteMesh ( const std::string& name );

  /// Virtual destructor
  virtual ~WriteMesh();

  /// Get the class name
  static std::string type_name () { return "WriteMesh"; }

  // functions specific to the WriteMesh component

  /// @name SIGNALS
  //@{

  /// signal to write the mesh
  void signal_write_mesh ( common::SignalArgs& node );
  /// signature of signal to write the mesh
  void signature_write_mesh ( common::SignalArgs& node);

  //@} END SIGNALS

  /// function to write the mesh
  /// @param fields selection of the fields of data to write
  void write_mesh( const CMesh&, const common::URI& file, const std::vector<common::URI>& fields);

  /// function to write the mesh
  /// writes all the fields on the mesh
  void write_mesh( const CMesh&, const common::URI& file);

  virtual void execute();

protected: // helper functions

  /// updates the list of avialable readers and regists each one to the extension it supports
  void update_list_of_available_writers();

private: // data

  std::map<std::string,std::vector<mesh::CMeshWriter::Ptr> > m_extensions_to_writers;

  boost::weak_ptr<CMesh> m_mesh;
  common::URI m_file;
  std::vector<common::URI> m_fields;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_WriteMesh_hpp
