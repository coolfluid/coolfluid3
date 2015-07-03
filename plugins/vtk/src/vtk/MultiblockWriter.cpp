// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <vtkMultiBlockDataSet.h>
#include <vtkXMLMultiBlockDataWriter.h>
#include <vtkUnstructuredGridWriter.h>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"

#include "vtk/MultiblockWriter.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace vtk {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < MultiblockWriter, mesh::MeshWriter, LibVTK> MultiblockWriter_Builder;

////////////////////////////////////////////////////////////////////////////////

MultiblockWriter::MultiblockWriter ( const std::string& name ) : MeshWriter(name)
{
  m_cf3_to_vtk = create_component<CF3ToVTK>("CF3ToVTK");
}

MultiblockWriter::~MultiblockWriter()
{
}

std::vector<std::string> MultiblockWriter::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".vtm");
  extensions.push_back(".vtmb");
  return extensions;
}

void MultiblockWriter::write()
{
  m_cf3_to_vtk->options().set("mesh", m_mesh);
  m_cf3_to_vtk->execute();
  vtkSmartPointer<vtkXMLMultiBlockDataWriter> writer = vtkSmartPointer<vtkXMLMultiBlockDataWriter>::New();
  cf3_assert(is_not_null(m_cf3_to_vtk->vtk_multiblock_set()));
  writer->SetInputData(m_cf3_to_vtk->vtk_multiblock_set());
  writer->SetFileName(m_file_path.path().c_str());
  if(writer->Write() != 1)
    CFerror << "Error writing mesh to " << m_file_path.path() << CFendl;
}

const mesh::Mesh& MultiblockWriter::mesh()
{
  if(is_null(m_mesh))
    throw common::SetupError(FromHere(), "Configured mesh for MeshWriter " + uri().path() + " is null");
  return *m_mesh;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace vtk
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////
