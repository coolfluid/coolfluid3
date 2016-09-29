// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_vtk_CF3ToVTK_hpp
#define CF_vtk_CF3ToVTK_hpp

#include <vtkSmartPointer.h>

#include "common/Action.hpp"
#include "mesh/Mesh.hpp"

#include "vtk/LibVTK.hpp"

////////////////////////////////////////////////////////////////////////////////

// VTK forward declarations
class vtkMultiBlockDataSet;

namespace cf3 {
namespace vtk {

////////////////////////////////////////////////////////////////////////////////

/// Convert a mesh to VTK format
class CF3ToVTK : public common::Action
{
public:
  CF3ToVTK ( const std::string& name );
  virtual ~CF3ToVTK();
  
  static std::string type_name () { return "CF3ToVTK"; }
  
  virtual void execute();

  // Return the created vtkMultiBlockDataSet
  vtkSmartPointer<vtkMultiBlockDataSet> vtk_multiblock_set()
  {
    return m_multiblock_set;
  }

  void reset();

private:
  Handle<mesh::Mesh const> m_mesh;
  vtkSmartPointer<vtkMultiBlockDataSet> m_multiblock_set;
  struct node_mapping;
  std::unique_ptr<node_mapping> m_node_mapping;
};
  
////////////////////////////////////////////////////////////////////////////////

} //  vtk
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif /* CF_vtk_CF3ToVTK_hpp */
