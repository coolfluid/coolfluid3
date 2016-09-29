// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_vtk_MultiblockWriter_hpp
#define CF_vtk_MultiblockWriter_hpp

#include "mesh/MeshWriter.hpp"

#include "vtk/LibVTK.hpp"
#include "vtk/CF3ToVTK.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace vtk {

////////////////////////////////////////////////////////////////////////////////

/// Write out a mesh using VTK multi-block format
class MultiblockWriter : public mesh::MeshWriter
{
public:
  MultiblockWriter ( const std::string& name );
  virtual ~MultiblockWriter();
  
  static std::string type_name () { return "MultiblockWriter"; }

  virtual std::string get_format() { return "VTKMultiBlock"; }

  virtual std::vector<std::string> get_extensions();
  
  virtual void write();

  const mesh::Mesh& mesh();

private:
  Handle<CF3ToVTK> m_cf3_to_vtk;
};
  
////////////////////////////////////////////////////////////////////////////////

} //  vtk
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif /* CF_vtk_MultiblockWriter_hpp */
