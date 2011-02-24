// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMeshTransformer_hpp
#define CF_Mesh_CMeshTransformer_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/CAction.hpp"

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {

  class CMesh;
  
////////////////////////////////////////////////////////////////////////////////

/// CMeshTransformer component class
/// This class serves as a component that that will operate on meshes
/// @author Willem Deconinck
class Mesh_API CMeshTransformer : public Common::CAction 
{
  
public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CMeshTransformer> Ptr;
  typedef boost::shared_ptr<CMeshTransformer const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMeshTransformer ( const std::string& name );

  /// Virtual destructor
  virtual ~CMeshTransformer();

  /// Get the class name
  static std::string type_name () { return "CMeshTransformer"; }

  // --------- Direct access ---------

  virtual void transform(boost::shared_ptr<CMesh> mesh);
  
  virtual void execute() = 0;
  
  /// brief description, typically one line
  virtual std::string brief_description() const = 0;

  /// extended help that user can query
  virtual std::string help() const = 0;

  void set_mesh(boost::shared_ptr<CMesh> mesh);

protected: // data
  
  boost::weak_ptr<CMesh> m_mesh;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshTransformer_hpp
