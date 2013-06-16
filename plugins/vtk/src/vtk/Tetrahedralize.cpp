// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <vtkCellType.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkIdList.h>
#include <vtkGenericCell.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCell.h>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/List.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Mesh.hpp"
#include <mesh/Faces.hpp>

#include "vtk/Tetrahedralize.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace vtk {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Tetrahedralize, mesh::MeshTransformer, LibVTK> Tetrahedralize_Builder;

////////////////////////////////////////////////////////////////////////////////

Tetrahedralize::Tetrahedralize ( const std::string& name ) : MeshTransformer ( name )
{
}

Tetrahedralize::~Tetrahedralize()
{
}

void Tetrahedralize::execute()
{
  const mesh::Field& coordinates = mesh().geometry_fields().coordinates();
  const Uint nb_points = coordinates.size();
  
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->Allocate(nb_points);
  BOOST_FOREACH(const mesh::Field::ConstRow coord, coordinates.array())
  {
    points->InsertNextPoint(coord[XX], coord[YY], coord[ZZ]);
  }
  
  vtkSmartPointer<vtkUnstructuredGrid> vtk_unstruc_grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  vtk_unstruc_grid->SetPoints(points);
  
  std::vector< Handle<common::Component> > to_remove;
  
  Handle<mesh::Region> parent_region;
  
  BOOST_FOREACH(mesh::Elements& elems, common::find_components_recursively_with_filter<mesh::Elements>(mesh(), mesh::IsElementsVolume()))
  {
    int cell_type = 0;
    mesh::GeoShape::Type shape = elems.element_type().shape();
    
    if(shape == mesh::GeoShape::TETRA)
      continue;
    
    if(shape == mesh::GeoShape::HEXA)
      cell_type = VTK_HEXAHEDRON;
    else if(shape == mesh::GeoShape::PRISM)
      cell_type = VTK_WEDGE;
    else
      throw common::NotImplemented(FromHere(), "Cells of type " + elems.element_type().name() + " can't be triangulated");
    
    to_remove.push_back(elems.handle());
    
    const mesh::Connectivity& connectivity = elems.geometry_space().connectivity();
    const Uint nb_elems = connectivity.size();
    const Uint nb_elem_nodes = connectivity.row_size();
    vtkSmartPointer<vtkIdList> id_list = vtkSmartPointer<vtkIdList>::New();
    id_list->SetNumberOfIds(nb_elem_nodes);
    for(Uint i = 0; i != nb_elems; ++i)
    {
      const mesh::Connectivity::ConstRow row = connectivity[i];
      for(int j = 0; j != nb_elem_nodes; ++j)
        id_list->SetId(j, row[j]);
      vtk_unstruc_grid->InsertNextCell(cell_type, id_list);
    }
    
    parent_region = common::find_parent_component_ptr<mesh::Region>(elems);
  }
  
  vtkSmartPointer<vtkDataSetTriangleFilter> triangulator = vtkSmartPointer<vtkDataSetTriangleFilter>::New();
  triangulator->SetTetrahedraOnly(true);
  triangulator->SetInput(vtk_unstruc_grid);
  
  vtkSmartPointer<vtkUnstructuredGrid> out_grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  triangulator->SetOutput(out_grid);
  triangulator->Update();
  
  cf3_assert(is_not_null(parent_region));
  
  CFinfo << "Adding tetrahedralization to region " << parent_region->uri().path() << CFendl;
  const Uint nb_tets = out_grid->GetNumberOfCells();
  Handle<mesh::Cells> tetras = parent_region->create_component<mesh::Cells>("Tetras");
  tetras->initialize("cf3.mesh.LagrangeP1.Tetra3D",mesh().geometry_fields());
  tetras->resize(nb_tets);
  common::Table<Uint>& connectivity = tetras->geometry_space().connectivity();
  vtkSmartPointer<vtkGenericCell> cell = vtkSmartPointer<vtkGenericCell>::New();
  std::vector< std::set<Uint> > connectivity_sets(nb_points);
  for(Uint i = 0; i != nb_tets; ++i)
  {
    mesh::Connectivity::Row row = connectivity[i];
    out_grid->GetCell(i, cell);
    for(Uint j = 0; j != 4; ++j)
    {
      row[j] = cell->GetPointId(j);
    }
    for(Uint j = 0; j != 4; ++j)
    {
      connectivity_sets[row[j]].insert(row.begin(), row.end());
    }
  }
  
  // Triangulate surface elements (quads only)
  BOOST_FOREACH(mesh::Elements& elems, common::find_components_recursively_with_filter<mesh::Elements>(mesh(), mesh::IsElementsSurface()))
  {
    if(elems.element_type().shape() == mesh::GeoShape::TRIAG)
      continue;
    
    if(elems.element_type().shape() != mesh::GeoShape::QUAD)
      throw common::NotImplemented(FromHere(), "Cells of type " + elems.element_type().name() + " can't be triangulated");
    
    to_remove.push_back(elems.handle());
    
    const mesh::Connectivity& connectivity = elems.geometry_space().connectivity();
    const Uint nb_quads = connectivity.size();
    
    Handle<mesh::Faces> triags = common::find_parent_component<mesh::Region>(elems).create_component<mesh::Faces>("TriangulatedFaces");
    triags->initialize("cf3.mesh.LagrangeP1.Triag3D",mesh().geometry_fields());
    triags->resize(nb_quads*2);
    mesh::Connectivity& triag_connectivity = triags->geometry_space().connectivity();
    
    for(Uint i = 0; i != nb_quads; ++i)
    {
      const mesh::Connectivity::ConstRow quad_row = connectivity[i];
      mesh::Connectivity::Row triag1_row = triag_connectivity[i*2];
      mesh::Connectivity::Row triag2_row = triag_connectivity[i*2+1];
      const mesh::Connectivity::ConstRow row = connectivity[i];
      if(connectivity_sets[row[0]].count(row[2]) > 0)
      {
        triag1_row[0] = quad_row[0]; triag1_row[1] = quad_row[1]; triag1_row[2] = quad_row[2];
        triag2_row[0] = quad_row[0]; triag2_row[1] = quad_row[2]; triag2_row[2] = quad_row[3];
      }
      else
      {
        cf3_assert(connectivity_sets[row[1]].count(row[3]) > 0);
        triag1_row[0] = quad_row[0]; triag1_row[1] = quad_row[1]; triag1_row[2] = quad_row[3];
        triag2_row[0] = quad_row[1]; triag2_row[1] = quad_row[2]; triag2_row[2] = quad_row[3];
      }
    }
  }
  
  // Remove all triangulated elements
  BOOST_FOREACH(const Handle<common::Component>& comp, to_remove)
  {
    comp->parent()->remove_component(*comp);
  }

  common::PE::Comm& comm = common::PE::Comm::instance();
  const Uint nb_procs = comm.size();
  const Uint rank = comm.rank();

  // Total number of elements on this rank
  Uint mesh_nb_elems = 0;
  BOOST_FOREACH(mesh::Elements& elements , common::find_components_recursively<mesh::Elements>(mesh()))
  {
    mesh_nb_elems += elements.size();
  }

  std::vector<Uint> nb_elements_accumulated;
  if(comm.is_active())
  {
    // Get the total number of elements on each rank
    comm.all_gather(mesh_nb_elems, nb_elements_accumulated);
  }
  else
  {
    nb_elements_accumulated.push_back(mesh_nb_elems);
  }
  cf3_assert(nb_elements_accumulated.size() == nb_procs);
  // Sum up the values
  for(Uint i = 1; i != nb_procs; ++i)
    nb_elements_accumulated[i] += nb_elements_accumulated[i-1];

  // Offset to start with for this rank
  Uint element_offset = rank == 0 ? 0 : nb_elements_accumulated[rank-1];

  // Update the element ranks and gids
  BOOST_FOREACH(mesh::Elements& elements , common::find_components_recursively<mesh::Elements>(mesh()))
  {
    const Uint nb_elems = elements.size();
    elements.rank().resize(nb_elems);
    elements.glb_idx().resize(nb_elems);

    for (Uint elem=0; elem != nb_elems; ++elem)
    {
      elements.rank()[elem] = rank;
      elements.glb_idx()[elem] = elem + element_offset;
    }
    element_offset += nb_elems;
  }

  mesh().update_statistics();
  mesh().update_structures();
  mesh().check_sanity();
}


////////////////////////////////////////////////////////////////////////////////

} // namespace vtk
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////
