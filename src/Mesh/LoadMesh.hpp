// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LoadMesh_hpp
#define CF_Mesh_LoadMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/URI.hpp"
#include "Mesh/CMeshReader.hpp"

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {
  class CMesh;
////////////////////////////////////////////////////////////////////////////////

/// @author Tiago Quintino
class Mesh_API LoadMesh : public Common::Component {

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

  /// Signal run_operation
  void signal_load_mesh ( Common::XmlNode& node );

  void signature_load_mesh ( Common::XmlNode& node);

  LoadMesh& operation(const std::string& name);

  boost::shared_ptr<CMesh> load_mesh(const Common::URI& file);
  
protected: // helper functions

  /// updates the list of avialable readers and regists each one to the extension it supports
  void update_list_of_available_readers();

private: // data

  std::map<std::string,std::vector<Mesh::CMeshReader::Ptr> > m_extensions_to_readers;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_LoadMesh_hpp
