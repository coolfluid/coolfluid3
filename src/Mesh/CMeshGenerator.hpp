// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CMeshGenerator_hpp
#define cf3_Mesh_CMeshGenerator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostFilesystem.hpp"

#include "Common/CAction.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"

namespace cf3 {
namespace Mesh {

  class CMesh;

////////////////////////////////////////////////////////////////////////////////

/// CMeshGenerator component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API CMeshGenerator : public common::CAction {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CMeshGenerator> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CMeshGenerator const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMeshGenerator ( const std::string& name );

  /// Virtual destructor
  virtual ~CMeshGenerator();

  /// Get the class name
  static std::string type_name () { return "CMeshGenerator"; }

  /// execute
  virtual void execute() = 0;

  /// generate, wraps execute() and returns the mesh reference
  CMesh& generate();

private: // functions

  void config_mesh();

protected: // functions

  void raise_mesh_loaded();

protected: // data

  boost::weak_ptr<CMesh> m_mesh;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Mesh_CMeshGenerator_hpp
