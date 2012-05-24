// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_FieldManager_hpp
#define cf3_mesh_FieldManager_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "common/Component.hpp"

#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

class Dictionary;
class Mesh;

////////////////////////////////////////////////////////////////////////////////

/// FieldManager component class
/// FieldManager can be used to simplify the creation of tagged fields, based on tagged VariablesDescriptors
/// @author Bart Janssens
class Mesh_API FieldManager : public common::Component {

public: // functions

  /// Contructor
  /// @param name of the component
  FieldManager ( const std::string& name );

  /// Virtual destructor
  virtual ~FieldManager();

  /// Get the class name
  static std::string type_name () { return "FieldManager"; }

  /// Create fields. Looks up the VariablesDescriptor with the given tag, and creates a field with the same tag in the given field group.
  void create_field(const std::string& tag, cf3::mesh::Dictionary& dict);

  /// @name SIGNALS
  //@{

  /// Creates the fields
  void signal_create_field( common::SignalArgs& node );

  //@} END SIGNALS

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_FieldManager_hpp
