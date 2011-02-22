// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CDomain_hpp
#define CF_Mesh_CDomain_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// CDomain component class
/// CDomain stores the meshes
/// @author Tiago Quintino
class Mesh_API CDomain : public Common::Component {

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

  /// Signal to load a mesh
  void signal_load_mesh ( Common::Signal::arg_t& node );

  void signature_load_mesh ( Common::Signal::arg_t& node);

  /// Signal to generate a mesh
  void signal_generate_mesh ( Common::Signal::arg_t& node );

  void signature_generate_mesh( Common::Signal::arg_t& node);

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CDomain_hpp
