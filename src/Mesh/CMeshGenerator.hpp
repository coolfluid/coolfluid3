// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMeshGenerator_hpp
#define CF_Mesh_CMeshGenerator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostFilesystem.hpp"

#include "Common/CAction.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

  class CMesh;

////////////////////////////////////////////////////////////////////////////////

/// CMeshGenerator component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API CMeshGenerator : public Common::CAction {

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
  virtual void execute() =0;

  static void mesh_loaded(CMesh& mesh);

protected: // data

  boost::weak_ptr<Component> m_parent;
  std::string m_name;


};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshGenerator_hpp
