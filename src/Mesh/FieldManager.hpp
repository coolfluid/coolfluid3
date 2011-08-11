// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_FieldManager_hpp
#define CF_Mesh_FieldManager_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "Common/Component.hpp"

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {

class FieldGroup;
class CMesh;

////////////////////////////////////////////////////////////////////////////////

/// FieldManager component class
/// FieldManager can be used to simplify the creation of tagged fields, based on tagged VariablesDescriptors
/// @author Bart Janssens
class Mesh_API FieldManager : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<FieldManager> Ptr;
  typedef boost::shared_ptr<FieldManager const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  FieldManager ( const std::string& name );

  /// Virtual destructor
  virtual ~FieldManager();

  /// Get the class name
  static std::string type_name () { return "FieldManager"; }

  /// Create fields. Looks up the VariablesDescriptor with the given tag, and creates a field with the same tag in the given field group.
  void create_field(const std::string& tag, CF::Mesh::FieldGroup& field_group);

  /// @name SIGNALS
  //@{

  /// Creates the fields
  void signal_create_field( Common::SignalArgs& node );

  //@} END SIGNALS

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_FieldManager_hpp
