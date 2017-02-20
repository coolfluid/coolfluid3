// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <queue>
#include <set>

#include "common/BoostAssign.hpp"

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkDoubleArray.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkGenericCell.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCell.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkDataObjectTreeIterator.h>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Log.hpp"
#include "common/List.hpp"
#include "common/OptionList.hpp"

#include "math/MatrixTypes.hpp"
#include "math/VariablesDescriptor.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Faces.hpp"

#include "mesh/LagrangeP1/ElementTypes.hpp"
#include "mesh/LagrangeP2/ElementTypes.hpp"

#include "vtk/CF3ToVTK.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace vtk {

namespace detail
{

template<typename ComponentT, typename BeginT, typename EndT>
void recurse(ComponentT& root, BeginT on_begin, EndT on_end)
{
  on_begin(root);
  BOOST_FOREACH(ComponentT& child, common::find_components<ComponentT>(root))
  {
    recurse(child, on_begin, on_end);
  }
  on_end(root);
}

template<typename ComponentT, typename BeginT>
void recurse(ComponentT& root, BeginT on_begin)
{
  recurse(root, on_begin, [](ComponentT&){});
}

// Get the VTK cell type corresponding to the given sf, or return -1 if it doesn't exist
int vtk_type(const mesh::ShapeFunction& sf)
{
  typedef std::map<mesh::GeoShape::Type, int> map_t;
  static const std::vector<map_t> element_type_map = boost::assign::list_of<map_t>
  (
    boost::assign::map_list_of // First order
      (mesh::LagrangeP1::Line::shape, VTK_LINE)
      (mesh::LagrangeP1::Hexa::shape, VTK_HEXAHEDRON)
      (mesh::LagrangeP1::Quad::shape, VTK_QUAD)
      (mesh::LagrangeP1::Tetra::shape, VTK_TETRA)
      (mesh::LagrangeP1::Triag::shape, VTK_TRIANGLE)
  )
  (
    boost::assign::map_list_of // Second order
      (mesh::LagrangeP2::Quad::shape, VTK_BIQUADRATIC_QUAD)
      (mesh::LagrangeP2::Line::shape, VTK_QUADRATIC_EDGE)
  );

  if(sf.order() == 0)
    return -1;

  if(sf.order() > element_type_map.size())
    return -1;

  const map_t map = element_type_map[sf.order()-1];
  const map_t::const_iterator it = map.find(sf.shape());
  if(it == map.end())
    return -1;

  return it->second;
}

typedef std::map< const mesh::Field*, std::vector< vtkSmartPointer<vtkDoubleArray> > > field_map_t;

// Add field arrays for the given dict
void add_field_arrays(const mesh::Dictionary& dict, const bool include_coords_field, const Uint nb_entries, field_map_t& field_map, vtkUnstructuredGrid& vtk_grid, const bool cell_data)
{
  std::map< std::string, vtkSmartPointer<vtkDoubleArray> > field_name_map; // Make sure field names are unique
  for(const Handle<mesh::Field>& field : dict.fields())
  {
    if(field->has_tag(mesh::Tags::coordinates()) && !include_coords_field)
    {
      continue;
    }
    auto& field_arrays = field_map[field.get()];
    field_arrays.clear();
    const math::VariablesDescriptor& descriptor = field->descriptor();
    const Uint nb_vars = descriptor.nb_vars();
    field_arrays.reserve(nb_vars);
    for(Uint var_idx = 0; var_idx != nb_vars; ++var_idx)
    {
      auto vtk_array = vtkSmartPointer<vtkDoubleArray>::New();
      std::string field_name = descriptor.user_variable_name(var_idx);
      Uint idx = 1;
      // Make sure the name is unique
      while(field_name_map.count(field_name) != 0)
      {
        field_name = descriptor.user_variable_name(var_idx) + "_" + common::to_str(idx);
        ++idx;
      }
      if(idx != 1)
        CFwarn << "Duplicate field name " << descriptor.user_variable_name(var_idx) << " was replaced with " << field_name << " for conversion to VTK" << CFendl;
      field_name_map[field_name] = vtk_array;
      vtk_array->SetNumberOfComponents(descriptor.dimensionality(descriptor.internal_variable_name(var_idx)) == math::VariablesDescriptor::Dimensionalities::VECTOR ? 3 : descriptor.var_length(var_idx));
      vtk_array->SetNumberOfTuples(nb_entries);
      vtk_array->SetName(field_name.c_str());
      if(cell_data)
      {
        vtk_grid.GetCellData()->AddArray(vtk_array);
      }
      else
      {
        vtk_grid.GetPointData()->AddArray(vtk_array);
      }

      field_arrays.push_back(vtk_array);
    }
  }
}

}

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CF3ToVTK, common::Action, LibVTK> CF3ToVTK_Builder;

////////////////////////////////////////////////////////////////////////////////

// Provide a mapping between global node indices and directly used nodes for a given region
struct CF3ToVTK::node_mapping
{
  // Key type for a region
  typedef std::pair<const mesh::Dictionary*, const mesh::Region*> region_key_t;
  typedef std::map<Uint, Uint> node_map_t;
  typedef detail::field_map_t field_map_t;

  node_mapping(const bool include_ghost_cells) : m_include_ghost_cells(include_ghost_cells)
  {
  }

  // Add a region to the vtk grid, building the structures needed to update field values at a later stage
  void add_region(vtkUnstructuredGrid& vtk_grid, const mesh::Dictionary& dict, const mesh::Region& region)
  {
    const Uint my_rank = common::PE::Comm::instance().rank();
    node_map_t& node_map = m_node_maps[std::make_pair(&dict, &region)];
    field_map_t& field_map = m_field_maps[std::make_pair(&dict, &region)];
    const bool is_geometry = dict.has_tag(mesh::Tags::geometry());

    // Allocate connectivity storage
    Uint nb_cells = 0;
    Uint nb_all_cells = 0;
    std::set<const mesh::Dictionary*> cell_dicts;
    for(const mesh::Entities& entities : common::find_components<mesh::Entities>(region))
    {
      if(detail::vtk_type(entities.space(dict).shape_function()) != -1)
      {
        nb_cells += (m_include_ghost_cells ? entities.size() : std::count_if(std::begin(entities.rank().array()), std::end(entities.rank().array()), [=](const Uint element_rank){return element_rank == my_rank;}));

        // Count the field arrays
        if(is_geometry)
        {
          for(const Handle<mesh::Space const>& space : entities.spaces())
          {
            if(!space->dict().continuous()) // skip continuous dict
            {
              cell_dicts.insert(&space->dict());
            }
          }
        }
      }

      nb_all_cells += entities.size();
    }
    vtk_grid.Allocate(nb_cells);

    if(is_geometry)
    {
      field_map_t& cell_field_map = m_cell_field_maps[&region];
      cell_field_map.clear();
      // Space to map global region element index to VTK index
      m_cell_maps[&region].resize(nb_all_cells, std::numeric_limits<Uint>::max());

      for(const auto dict : cell_dicts)
      {
        detail::add_field_arrays(*dict, m_include_coords_field, nb_cells, cell_field_map, vtk_grid, true);
      }
    }


    // Keep track of what nodes have been added
    const Uint unused_node = std::numeric_limits<Uint>::max();
    std::vector<Uint> cf3_node_to_vtk(dict.size(), unused_node);
    std::vector<Uint>& cf3_cell_to_vtk = m_cell_maps[&region];
    Uint vtk_point_idx = 0; // Last index of the vtk points
    Uint vtk_cell_idx = 0; // Last VTK cell added
    Uint cf3_cell_idx = 0;

    // Add connectivity data and build node map
    for(const mesh::Entities& entities : common::find_components<mesh::Entities>(region))
    {
      const Uint nb_elements = entities.size();
      const mesh::Space& space = entities.space(dict);
      const int vtk_cell_type = detail::vtk_type(space.shape_function());
      if(vtk_cell_type == -1)
      {
        CFwarn << "skipping unsupported element type " << space.shape_function().shape_name() << " from space " << space.uri().path() << " in conversion to VTK" << CFendl;
        continue;
      }
      const mesh::Connectivity& connectivity = space.connectivity();
      const Uint nb_element_nodes = connectivity.row_size();
      vtkSmartPointer<vtkIdList> id_list = vtkSmartPointer<vtkIdList>::New();
      id_list->SetNumberOfIds(nb_element_nodes);
      for(Uint elem_idx = 0; elem_idx != nb_elements; ++elem_idx)
      {
        if(entities.is_ghost(elem_idx) && !m_include_ghost_cells)
          continue;

        const mesh::Connectivity::ConstRow row = connectivity[elem_idx];
        for(int j = 0; j != nb_element_nodes; ++j)
        {
          const Uint node_idx = row[j];
          if(cf3_node_to_vtk[node_idx] == unused_node)
          {
            node_map[node_idx] = vtk_point_idx;
            cf3_node_to_vtk[node_idx] = vtk_point_idx;
            ++vtk_point_idx;
          }
          id_list->SetId(j, cf3_node_to_vtk[node_idx]);
        }
        vtk_grid.InsertNextCell(detail::vtk_type(space.shape_function()), id_list);
        if(is_geometry)
        {
          cf3_assert(cf3_cell_idx < cf3_cell_to_vtk.size());
          cf3_cell_to_vtk[cf3_cell_idx] = vtk_cell_idx;
        }
        ++vtk_cell_idx;
        ++cf3_cell_idx;
      }
    }

    // Add points
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(node_map.size());
    const mesh::Field& coordinates = dict.coordinates();
    Real coord[3] = {0.,0.,0.};
    for(const auto& mapped_node : node_map)
    {
      const mesh::Field::ConstRow coord_row = coordinates[mapped_node.first];
      std::copy(std::begin(coord_row), std::end(coord_row), std::begin(coord));
      points->SetPoint(mapped_node.second, coord);
    }
    vtk_grid.SetPoints(points);

    // Add point attribute arrays
    detail::add_field_arrays(dict, m_include_coords_field, node_map.size(), field_map, vtk_grid, false);
  }

  void update_field_values()
  {
    for(auto& field_map_kv : m_field_maps)
    {
      const node_map_t node_map = m_node_maps[field_map_kv.first];
      for(auto& field_vars : field_map_kv.second)
      {
        const mesh::Field::ArrayT& source_array = field_vars.first->array();
        std::vector< vtkSmartPointer<vtkDoubleArray> >& arrays = field_vars.second;
        const Uint nb_arrays = arrays.size();
        const math::VariablesDescriptor& descriptor = field_vars.first->descriptor();
        for(const auto& node_link : node_map)
        {
          const mesh::Field::ConstRow row = source_array[node_link.first];
          for(Uint array_idx = 0; array_idx != nb_arrays; ++array_idx)
          {
            const Uint nb_comps = descriptor.var_length(array_idx);
            const Uint offset = descriptor.offset(array_idx);
            for(Uint comp_idx = 0; comp_idx != nb_comps; ++comp_idx)
            {
              arrays[array_idx]->SetTypedComponent(node_link.second, comp_idx, row[offset+comp_idx]);
            }
          }
        }
      }
    }
    for(const auto& cell_map_kv : m_cell_maps)
    {
      const mesh::Region& region = *cell_map_kv.first;
      const std::vector<Uint>& cf3_cell_to_vtk = cell_map_kv.second;
      field_map_t& field_map = m_cell_field_maps[&region];
      for(auto& field_vars : field_map)
      {
        const mesh::Field& field = *field_vars.first;
        const Uint row_size = field.row_size();
        const mesh::Field::ArrayT& source_array = field.array();
        const math::VariablesDescriptor& descriptor = field.descriptor();
        std::vector< vtkSmartPointer<vtkDoubleArray> >& arrays = field_vars.second;
        const Uint nb_arrays = arrays.size();
        Uint cf3_cell_idx = 0;
        for(const mesh::Entities& entities : common::find_components<mesh::Entities>(region))
        {
          const Uint nb_elements = entities.size();
          const mesh::Connectivity& connectivity = entities.space(field.dict()).connectivity();
          const Uint elem_nb_nodes = connectivity.row_size();
          RealVector row_sum(row_size);
          for(Uint elem_idx = 0; elem_idx != nb_elements; ++elem_idx)
          {
            if(entities.is_ghost(elem_idx) && !m_include_ghost_cells)
              continue;

            row_sum.setZero();
            for(const Uint node_idx : connectivity[elem_idx])
            {
              row_sum += Eigen::Map<RealVector const>(&source_array[node_idx][0], row_size);
            }
            row_sum /= static_cast<Real>(elem_nb_nodes);

            for(Uint array_idx = 0; array_idx != nb_arrays; ++array_idx)
            {
              cf3_assert(cf3_cell_idx < cf3_cell_to_vtk.size());
              const Uint nb_comps = descriptor.var_length(array_idx);
              const Uint offset = descriptor.offset(array_idx);
              for(Uint comp_idx = 0; comp_idx != nb_comps; ++comp_idx)
              {
                arrays[array_idx]->SetTypedComponent(cf3_cell_to_vtk[cf3_cell_idx], comp_idx, row_sum[offset+comp_idx]);
              }
            }

            ++cf3_cell_idx;
          }
        }
      }
    }
  }

  std::map<region_key_t, node_map_t> m_node_maps;
  std::map<region_key_t, field_map_t> m_field_maps;
  std::map< const mesh::Region*, std::vector<Uint> > m_cell_maps;
  std::map< const mesh::Region*, field_map_t > m_cell_field_maps;
  const bool m_include_ghost_cells;
  bool m_include_coords_field = false;
};

CF3ToVTK::CF3ToVTK ( const std::string& name ) : common::Action ( name )
{
  options().add("mesh", m_mesh)
    .description("Mesh to convert")
    .pretty_name("Mesh")
    .link_to(&m_mesh)
    .attach_trigger(boost::bind(&CF3ToVTK::reset, this))
    .mark_basic();

  options().add("include_ghost_elements", false)
    .pretty_name("Include ghost elements")
    .description("Include ghost elements in the target VTK mesh")
    .attach_trigger(boost::bind(&CF3ToVTK::reset, this))
    .mark_basic();
}

CF3ToVTK::~CF3ToVTK()
{
}

void CF3ToVTK::reset()
{
  m_node_mapping.reset();
}

void CF3ToVTK::execute()
{
  if(is_null(m_mesh))
    throw common::SetupError(FromHere(), "Mesh is not set for CF3ToVTK");

  if(m_node_mapping == nullptr)
  {
    m_node_mapping.reset(new node_mapping(options().value<bool>("include_ghost_elements")));

    const mesh::Mesh& mesh = *m_mesh;

    const bool include_ghost_elements = options().value<bool>("include_ghost_elements");
    m_multiblock_set = vtkSmartPointer<vtkMultiBlockDataSet>::New();

    const Uint nb_dicts = mesh.dictionaries().size();
    const Uint nb_blocks_with_geom = std::count_if(mesh.dictionaries().begin(), mesh.dictionaries().end(), [](const Handle<mesh::Dictionary const>& dict) {return dict->continuous() && (dict->get_child(mesh::Tags::coordinates()) != nullptr);} );
    m_multiblock_set->SetNumberOfBlocks(nb_blocks_with_geom);

    Uint added_block_idx = 0;
    for(Uint dict_idx = 0; dict_idx != nb_dicts; ++dict_idx)
    {
      const mesh::Dictionary& dict = *mesh.dictionaries()[dict_idx];
      if(dict.continuous())
      {
        if (is_null(dict.get_child(mesh::Tags::coordinates())))
        {
          CFwarn << "skipping continuous dictionary " << dict.name() << " because it has no coordinates" << CFendl;
          continue;
        }

        // Add the connectivity data
        std::stack< vtkSmartPointer<vtkMultiBlockDataSet> > dict_tree;
        std::stack<Uint> nb_blocks_stack;
        dict_tree.push(vtkSmartPointer<vtkMultiBlockDataSet>::New());
        dict_tree.top()->SetNumberOfBlocks(1);
        m_multiblock_set->SetBlock(added_block_idx, dict_tree.top());
        m_multiblock_set->GetMetaData(added_block_idx)->Set(vtkMultiBlockDataSet::NAME(), dict.name().c_str());
        ++added_block_idx;
        nb_blocks_stack.push(0);
        detail::recurse(mesh.topology(),
        [&](const mesh::Region& region) // Region begin
        {
          const Uint nb_regions = common::find_components<mesh::Region>(region).size();
          cf3_assert(!dict_tree.empty());

          vtkSmartPointer<vtkMultiBlockDataSet> parent = dict_tree.top();

          vtkSmartPointer<vtkMultiBlockDataSet> current_multiblock_set = vtkSmartPointer<vtkMultiBlockDataSet>::New();
          parent->SetBlock(nb_blocks_stack.top(), current_multiblock_set.Get());
          parent->GetMetaData(nb_blocks_stack.top()++)->Set(vtkMultiBlockDataSet::NAME(), region.name().c_str());
          current_multiblock_set->SetNumberOfBlocks(nb_regions);
          nb_blocks_stack.push(0);
          dict_tree.push(current_multiblock_set);

          vtkSmartPointer<vtkDataObject> current_data_object = current_multiblock_set;
          vtkSmartPointer<vtkUnstructuredGrid> current_grid;

          // Place elements in an unstructured grid
          if(common::find_components<mesh::Entities>(region).size() != 0)
          {
            current_grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
            current_multiblock_set->SetBlock(nb_blocks_stack.top()++, current_grid.Get());
            m_node_mapping->add_region(*current_grid, dict, region);
          }
        },
        [&](const mesh::Region& region) // Region end
        {
          dict_tree.pop();
          nb_blocks_stack.pop();
        }
        );
      }
    }
  }

  m_node_mapping->update_field_values();
}


////////////////////////////////////////////////////////////////////////////////

} // namespace vtk
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////
