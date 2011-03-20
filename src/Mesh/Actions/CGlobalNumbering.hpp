// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CGlobalNumbering_hpp
#define CF_Mesh_CGlobalNumbering_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that finds matching nodes in given regions of the mesh
/// @author Willem Deconinck
class Mesh_Actions_API CGlobalNumbering : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CGlobalNumbering> Ptr;
    typedef boost::shared_ptr<CGlobalNumbering const> ConstPtr;

public: // functions
  
  /// constructor
  CGlobalNumbering( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CGlobalNumbering"; }

  virtual void execute();
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;
  
private: // functions
  
  std::size_t hash_value(const RealVector& coords);
  std::size_t hash_value(const RealMatrix& coords);

  bool m_debug;
}; // end CGlobalNumbering


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CGlobalNumbering_hpp
