// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Actions_LoadBalance_hpp
#define CF_Mesh_Actions_LoadBalance_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Load Balance the mesh
///
/// @post After this, the mesh is ready to be parallellized
/// @author Willem Deconinck
class Mesh_Actions_API LoadBalance : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<LoadBalance> Ptr;
    typedef boost::shared_ptr<LoadBalance const> ConstPtr;

public: // functions

  /// constructor
  LoadBalance( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "LoadBalance"; }

  virtual void execute();

private:

  boost::shared_ptr<CMeshTransformer> m_partitioner;

}; // end LoadBalance


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Actions_LoadBalance_hpp
