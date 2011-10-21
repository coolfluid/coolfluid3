// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CDomain_hpp
#define cf3_mesh_CDomain_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "common/Component.hpp"
#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

  class CMesh;

////////////////////////////////////////////////////////////////////////////////

/// CDomain component class
/// CDomain stores the meshes and contains a link to the "active" mesh
/// @author Tiago Quintino
class Mesh_API CDomain : public common::Component {

public: // typedefs

  typedef boost::shared_ptr<CDomain> Ptr;
  typedef boost::shared_ptr<CDomain const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CDomain ( const std::string& name );

  /// Virtual destructor
  virtual ~CDomain();

  /// Get the class name
  static std::string type_name () { return "CDomain"; }

  /// loads the mesh
  /// @post mesh will be (automatically) load balanced in case of parallel run
  CMesh& load_mesh ( const common::URI& file, const std::string& name );
  
  /// write the active mesh
  void write_mesh(const common::URI& file);

  /// @name SIGNALS
  //@{
    
  /// Signal to load a mesh
  void signal_load_mesh( common::SignalArgs& node );
  
  /// Signal to write the active mesh
  void signal_write_mesh( common::SignalArgs& node );
  
  //@} END SIGNALS

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CDomain_hpp
