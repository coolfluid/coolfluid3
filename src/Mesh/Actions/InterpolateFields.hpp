// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Actions_InterpolateFields_hpp
#define CF_Mesh_Actions_InterpolateFields_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  class COcttree;
  class Field;

namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Interpolate Fields with matching support
///
/// @post After this, the mesh is ready to be parallellized
/// @author Willem Deconinck
class Mesh_Actions_API InterpolateFields : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<InterpolateFields> Ptr;
    typedef boost::shared_ptr<InterpolateFields const> ConstPtr;

public: // functions

  /// constructor
  InterpolateFields( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "InterpolateFields"; }

  virtual void execute();

private:

  /// source field
  boost::weak_ptr<Field> m_source;

  /// target field
  boost::weak_ptr<Field> m_target;

  /// source octtree
  boost::shared_ptr<COcttree> m_octtree;

}; // end InterpolateFields


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Actions_InterpolateFields_hpp
