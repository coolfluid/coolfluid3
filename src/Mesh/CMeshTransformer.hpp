// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMeshTransformer_hpp
#define CF_Mesh_CMeshTransformer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostFilesystem.hpp"

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
  virtual void transform(CMesh& mesh);
  
  virtual void execute();

  /// extended help that user can query
  virtual std::string help() const;

  void set_mesh(boost::shared_ptr<CMesh> mesh);
  void set_mesh(CMesh& mesh);

protected: // data
  
  boost::weak_ptr<CMesh> m_mesh;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshTransformer_hpp
