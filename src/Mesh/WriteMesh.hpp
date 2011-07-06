// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_WriteMesh_hpp
#define CF_Mesh_WriteMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CAction.hpp"
#include "Common/URI.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {
  class CMesh;
////////////////////////////////////////////////////////////////////////////////

/// @author Tiago Quintino
class Mesh_API WriteMesh : public Common::CAction {

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
  void signal_write_mesh ( Common::SignalArgs& node );
  /// signature of signal to write the mesh
  void signature_write_mesh ( Common::SignalArgs& node);

  //@} END SIGNALS

  /// function to write the mesh
  /// @param fields selection of the fields of data to write
  void write_mesh( const CMesh&, const Common::URI& file, const std::vector<Common::URI>& fields);

  /// function to write the mesh
  /// writes all the fields on the mesh
  void write_mesh( const CMesh&, const Common::URI& file);

  virtual void execute();

protected: // helper functions

  /// updates the list of avialable readers and regists each one to the extension it supports
  void update_list_of_available_writers();

private: // data

  std::map<std::string,std::vector<Mesh::CMeshWriter::Ptr> > m_extensions_to_writers;

  boost::weak_ptr<CMesh> m_mesh;
  Common::URI m_file;
  std::vector<Common::URI> m_fields;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_WriteMesh_hpp
