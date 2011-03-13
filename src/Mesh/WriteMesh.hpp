// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_WriteMesh_hpp
#define CF_Mesh_WriteMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/URI.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {
  class CMesh;
////////////////////////////////////////////////////////////////////////////////

/// @author Tiago Quintino
class Mesh_API WriteMesh : public Common::Component {

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

  /// Signal run_operation
  void signal_write_mesh ( Common::SignalArgs& node );

  void signature_write_mesh ( Common::SignalArgs& node);

  WriteMesh& operation(const std::string& name);

  void write_mesh( CMesh&, const Common::URI& file, const std::vector<Common::URI>& fields);

protected: // helper functions

  /// updates the list of avialable readers and regists each one to the extension it supports
  void update_list_of_available_writers();

private: // data

  std::map<std::string,std::vector<Mesh::CMeshWriter::Ptr> > m_extensions_to_writers;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_WriteMesh_hpp
