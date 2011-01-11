// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Actions_CExtract_hpp
#define CF_Mesh_Actions_CExtract_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class Mesh_Actions_API CExtract : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CExtract> Ptr;
    typedef boost::shared_ptr<CExtract const> ConstPtr;

private: // typedefs
  
public: // functions
  
  /// constructor
  CExtract( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CExtract"; }

  virtual void transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args);
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;

private: // data

  CMesh::Ptr m_mesh;
  
}; // end CExtract


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CExtract_hpp
