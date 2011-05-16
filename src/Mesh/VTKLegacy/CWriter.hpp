// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_VTKLegacy_CWriter_hpp
#define CF_Mesh_VTKLegacy_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/GeoShape.hpp"

#include "Mesh/VTKLegacy/LibVTKLegacy.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class ElementType;
namespace VTKLegacy {

//////////////////////////////////////////////////////////////////////////////

/// This class defines VTKLegacy mesh format writer
/// @author Bart Janssens
class VTKLegacy_API CWriter : public CMeshWriter
{
public: // typedefs

    typedef boost::shared_ptr<CWriter> Ptr;
    typedef boost::shared_ptr<CWriter const> ConstPtr;

public: // functions

  /// constructor
  CWriter( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CWriter"; }

  virtual void write_from_to(const CMesh& mesh, const Common::URI& file);

  virtual std::string get_format() { return "VTKLegacy"; }

  virtual std::vector<std::string> get_extensions();
}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // VTKLegacy
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_VTKLegacy_CWriter_hpp
