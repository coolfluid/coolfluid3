// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMeshExtract_hpp
#define CF_Mesh_CMeshExtract_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshTransformer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class Mesh_API CMeshExtract : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CMeshExtract> Ptr;
    typedef boost::shared_ptr<CMeshExtract const> ConstPtr;

private: // typedefs
  
public: // functions
  
  /// constructor
  CMeshExtract( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CMeshExtract"; }

  static void define_config_properties () {}

  virtual void transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args);
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;
  
private: // functions
 
  
private: // helper functions

  /// regists all the signals declared in this class
  virtual void define_signals () {}

private: // data

  CMesh::Ptr m_mesh;
  
}; // end CMeshExtract


////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshExtract_hpp
