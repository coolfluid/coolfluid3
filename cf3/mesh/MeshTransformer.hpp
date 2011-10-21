// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_MeshTransformer_hpp
#define cf3_mesh_MeshTransformer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/BoostFilesystem.hpp"

#include "common/Action.hpp"

#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

  class Mesh;

////////////////////////////////////////////////////////////////////////////////

/// MeshTransformer component class
/// This class serves as a component that that will operate on meshes
/// @author Willem Deconinck
class Mesh_API MeshTransformer : public common::Action
{

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<MeshTransformer> Ptr;
  typedef boost::shared_ptr<MeshTransformer const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  MeshTransformer ( const std::string& name );

  /// Virtual destructor
  virtual ~MeshTransformer();

  /// Get the class name
  static std::string type_name () { return "MeshTransformer"; }

  // --------- Direct access ---------

  virtual void transform(boost::shared_ptr<Mesh> mesh);
  virtual void transform(Mesh& mesh);

  virtual void execute();

  /// extended help that user can query
  virtual std::string help() const;

  void set_mesh(boost::shared_ptr<Mesh> mesh);
  void set_mesh(Mesh& mesh);

  Mesh& mesh()
  {
    cf3_assert(m_mesh.expired() == false);
    return *m_mesh.lock();
  }

  const Mesh& mesh() const
  {
    cf3_assert(m_mesh.expired() == false);
    return *m_mesh.lock();
  }

protected: // data

  boost::weak_ptr<Mesh> m_mesh;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MeshTransformer_hpp
