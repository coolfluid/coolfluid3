// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CGlobalConnectivity_hpp
#define cf3_Mesh_CGlobalConnectivity_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
namespace Actions {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that finds matching nodes in given regions of the mesh
/// @author Willem Deconinck
class Mesh_Actions_API CGlobalConnectivity : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CGlobalConnectivity> Ptr;
    typedef boost::shared_ptr<CGlobalConnectivity const> ConstPtr;

public: // functions
  
  /// constructor
  CGlobalConnectivity( const std::string& name );
  
  virtual ~CGlobalConnectivity();
  
  /// Gets the Class name
  static std::string type_name() { return "CGlobalConnectivity"; }

  virtual void execute();
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;


}; // end CGlobalConnectivity


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Mesh_CGlobalConnectivity_hpp
