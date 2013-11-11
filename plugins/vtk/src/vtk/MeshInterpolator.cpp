// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <vtkCellType.h>
#include <vtkIdList.h>
#include <vtkGenericCell.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCell.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProbeFilter.h>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "common/PE/Comm.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/BoundingBox.hpp"

#include "vtk/MeshInterpolator.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace vtk {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < MeshInterpolator, common::Action, LibVTK> MeshInterpolator_Builder;

////////////////////////////////////////////////////////////////////////////////

MeshInterpolator::MeshInterpolator ( const std::string& name ) : common::Action ( name )
{
  options().add("source_mesh", Handle<mesh::Mesh>())
    .pretty_name("Source Mesh")
    .description("Mesh to interpolate from. Warning: this is modified in parallel to make all elements known on each rank.")
    .mark_basic();
    
  options().add("target_mesh", Handle<mesh::Mesh>())
    .pretty_name("Target Mesh")
    .description("Mesh to interpolate to")
    .mark_basic();
    
  create_static_component<mesh::BoundingBox>("BoundingBox");
}

MeshInterpolator::~MeshInterpolator()
{
}

void MeshInterpolator::execute()
{
  
  Handle<mesh::Mesh> source_mesh = options().value< Handle<mesh::Mesh> >("source_mesh");
  Handle<mesh::Mesh> target_mesh = options().value< Handle<mesh::Mesh> >("target_mesh");
  
  if(is_null(source_mesh))
    throw common::SetupError(FromHere(), "MeshInterpolator source_mesh is not set");
  
  if(is_null(target_mesh))
    throw common::SetupError(FromHere(), "MeshInterpolator target_mesh is not set");
  
  if(source_mesh->dimension() != 3)
    throw common::SetupError(FromHere(), "VTK MeshInterpolator only works in 3D");
  
  Handle<mesh::Dictionary> source_dict(source_mesh->geometry_fields().handle());
  Handle<mesh::Dictionary> target_dict(target_mesh->geometry_fields().handle());
  
  common::PE::Comm& comm = common::PE::Comm::instance();
  const Uint nb_procs = comm.size();
  const Uint my_rank = comm.rank();
  
  const Uint nb_target_points = target_dict->size();
  
  Handle<mesh::BoundingBox> bounding_box(get_child("BoundingBox"));
  cf3_assert(is_not_null(bounding_box));
  
  const Uint dim = source_mesh->dimension();
  
  if(comm.size() > 1)
  {
    bounding_box->build(*target_mesh);
    std::vector<Real> my_bbox(dim*2);
    for(Uint i = 0; i != dim; ++i)
    {
      my_bbox[i] = bounding_box->min()[i];
      my_bbox[dim+i] = bounding_box->max()[i];
    }
    
    std::vector< std::vector<Real> > recv_bbox;
    comm.all_gather(my_bbox, recv_bbox);
    const Uint nb_ranks = comm.size();
    cf3_assert(recv_bbox.size() == nb_ranks);
    std::vector<RealVector> box_mins(nb_ranks);
    std::vector<RealVector> box_maxs(nb_ranks);
    for(Uint rank = 0; rank != nb_ranks; ++rank)
    {      
      const std::vector<Real>& other_bbox = recv_bbox[rank];
      const Eigen::Map<RealVector const> b_min(&other_bbox[0], dim);
      const Eigen::Map<RealVector const> b_max(&other_bbox[dim], dim);
      box_mins[rank] = b_min;
      box_maxs[rank] = b_max;
    }
    
    const mesh::Field& source_coords = source_dict->coordinates();
    
    mesh::MeshAdaptor adaptor(*source_mesh);
    adaptor.prepare();
    std::vector< std::vector< std::vector<Uint> > > elements_to_send(nb_procs, std::vector< std::vector<Uint> > (source_mesh->elements().size()));
    BOOST_FOREACH(mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(source_mesh->topology(), mesh::IsElementsVolume()))
    {
      const mesh::Connectivity& conn = elements.geometry_space().connectivity();
      const Uint nb_elems = elements.size();
      for(Uint elem_idx = 0; elem_idx != nb_elems; ++elem_idx)
      {
        for(Uint rank = 0; rank != nb_ranks; ++rank)
        {
          if(rank == comm.rank())
            continue;
          BOOST_FOREACH(const Uint node_idx, conn[elem_idx])
          {
            const Eigen::Map<RealVector const> coord(&source_coords[node_idx][0], dim);
            if((coord.array() <= box_maxs[rank].array()).all() && (coord.array() >= box_mins[rank].array()).all())
            {
              elements_to_send[rank][elements.entities_idx()].push_back(elem_idx);
              break;
            }
          }
        }
      }
    }

    std::vector< std::vector< std::vector<Uint> > > nodes_to_send;
    adaptor.find_nodes_to_export(elements_to_send,nodes_to_send);

    std::vector< std::vector< std::vector<boost::uint64_t> > > imported_elements;
    adaptor.send_elements(elements_to_send, imported_elements);

    adaptor.flush_elements();

    std::vector< std::vector< std::vector<boost::uint64_t> > > imported_nodes;
    adaptor.send_nodes(nodes_to_send,imported_nodes);

    adaptor.flush_nodes();

    adaptor.finish();
  }
  
  const Uint nb_source_points = source_dict->size();
  const mesh::Field& source_coords = source_dict->coordinates();
  
  // Build the input mesh
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->Allocate(nb_source_points);
  BOOST_FOREACH(const mesh::Field::ConstRow coord, source_coords.array())
  {
    points->InsertNextPoint(coord[XX], coord[YY], coord[ZZ]);
  }
  
  vtkSmartPointer<vtkUnstructuredGrid> vtk_unstruc_grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  vtk_unstruc_grid->SetPoints(points);
  
  Real nb_scalars = 0;
  std::vector<mesh::Field*> source_fields, target_fields;
  BOOST_FOREACH(const Handle<mesh::Field>& source_field, source_dict->fields())
  {
    if(source_field->has_tag(mesh::Tags::coordinates()))
      continue;

    Handle<mesh::Field> target_field(target_dict->get_child(source_field->name()));
    if(is_null(target_field))
    {
      target_field = target_dict->create_field(source_field->name(), source_field->descriptor().description()).handle<mesh::Field>();
      BOOST_FOREACH(const std::string& tag, source_field->get_tags())
      {
        target_field->add_tag(tag);
      }
    }
    nb_scalars += source_field->row_size();
    
    source_fields.push_back(source_field.get());
    target_fields.push_back(target_field.get());
  }
  
  vtkSmartPointer<vtkDoubleArray> source_data = vtkSmartPointer<vtkDoubleArray>::New();
  source_data->SetNumberOfComponents(nb_scalars);
  source_data->SetName("SourceData");
  source_data->Allocate(nb_source_points);
  std::vector<Real> field_values_row(nb_scalars);
  for(Uint i = 0; i != nb_source_points; ++i)
  {
    int offset = 0;
    BOOST_FOREACH(const mesh::Field* source_field, source_fields)
    {
      const mesh::Field::ConstRow fd_row = source_field->array()[i];
      std::copy(fd_row.begin(), fd_row.end(), field_values_row.begin()+offset);
      offset += source_field->row_size();
    }
    source_data->InsertNextTuple(&field_values_row[0]);
  }
  vtk_unstruc_grid->GetPointData()->SetScalars(source_data);
  
  BOOST_FOREACH(mesh::Elements& elems, common::find_components_recursively_with_filter<mesh::Elements>(*source_mesh, mesh::IsElementsVolume()))
  {
    int cell_type = 0;
    mesh::GeoShape::Type shape = elems.element_type().shape();
    
    if(shape == mesh::GeoShape::TETRA)
      cell_type = VTK_TETRA;
    else if(shape == mesh::GeoShape::HEXA)
      cell_type = VTK_HEXAHEDRON;
    else if(shape == mesh::GeoShape::PRISM)
      cell_type = VTK_WEDGE;
    else
      throw common::NotImplemented(FromHere(), "Cells of type " + elems.element_type().name() + " are not supported as interpolation source");
    
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
  }
  
  // Build the probing mesh
  vtkSmartPointer<vtkPolyData> probe_poly = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> probe_points = vtkSmartPointer<vtkPoints>::New();
  probe_points->Allocate(nb_target_points);
  BOOST_FOREACH(const mesh::Field::ConstRow coord, target_dict->coordinates().array())
  {
    probe_points->InsertNextPoint(coord[XX], coord[YY], coord[ZZ]);
  }
  probe_poly->SetPoints(probe_points);
  
  // Probe at the point locations of the probing mesh
  vtkSmartPointer<vtkProbeFilter> prober = vtkSmartPointer<vtkProbeFilter>::New();
  prober->SetInput(probe_poly);
  prober->SetSource(vtk_unstruc_grid);
  prober->Update();

  if(is_null(prober->GetOutput()))
    throw common::SetupError(FromHere(), "Mesh interpolation failed");

  vtkDataArray* interpolated_scalars = prober->GetOutput()->GetPointData()->GetScalars();
  cf3_assert(is_not_null(interpolated_scalars));

  // Store the field data
  for(Uint i = 0; i != nb_target_points; ++i)
  {
    int offset = 0;
    interpolated_scalars->GetTuple(i, &field_values_row[0]);
    BOOST_FOREACH(mesh::Field* target_field, target_fields)
    {
      mesh::Field::Row fd_row = target_field->array()[i];
      std::copy(field_values_row.begin()+offset, field_values_row.begin()+offset+target_field->row_size(), fd_row.begin());
      offset += target_field->row_size();
    }
  }
  
  BOOST_FOREACH(mesh::Field* target_field, target_fields)
  {
    target_field->parallelize();
    target_field->synchronize();
  }
}


////////////////////////////////////////////////////////////////////////////////

} // namespace vtk
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////
