// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_VTKLegacy_Writer_hpp
#define cf3_mesh_VTKLegacy_Writer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshWriter.hpp"
#include "mesh/GeoShape.hpp"

#include "mesh/VTKLegacy/LibVTKLegacy.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  class ElementType;
namespace VTKLegacy {

//////////////////////////////////////////////////////////////////////////////

/// This class defines VTKLegacy mesh format writer
/// @author Bart Janssens
class VTKLegacy_API Writer : public MeshWriter
{
public: // typedefs

    
    

public: // functions

  /// constructor
  Writer( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Writer"; }

  virtual void write();

  virtual std::string get_format() { return "VTKLegacy"; }

  virtual std::vector<std::string> get_extensions();
}; // end Writer


////////////////////////////////////////////////////////////////////////////////

} // VTKLegacy
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_VTKLegacy_Writer_hpp
