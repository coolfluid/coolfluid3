// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <queue>

#include <boost/assign/list_of.hpp>

#include <vtkCellType.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkGenericCell.h>
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
  static const std::vector<map_t> element_type_map = boost::assign::list_of<map_t>(
    boost::assign::map_list_of
      (mesh::LagrangeP1::Hexa::shape, VTK_HEXAHEDRON)
      (mesh::LagrangeP1::Quad::shape, VTK_QUAD)
  );

  if(sf.order() > element_type_map.size())
    return -1;

  const map_t map = element_type_map[sf.order()-1];
  const map_t::const_iterator it = map.find(sf.shape());
  if(it == map.end())
    return -1;

  return it->second;
}

}

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CF3ToVTK, common::Action, LibVTK> CF3ToVTK_Builder;

////////////////////////////////////////////////////////////////////////////////

// Provide a mapping between global node indices and directly used nodes for a given region
struct CF3ToVTK::node_mapping
{
  // Key type for a region
  typedef std::pair<const mesh::Dictionary*, const mesh::Region*> key_t;
  typedef std::map<Uint, Uint> node_map_t;

  node_mapping(const bool include_ghost_cells) : m_include_ghost_cells(include_ghost_cells)
  {
  }

  // Add a region to the vtk grid, building the structures needed to update field values at a later stage
  void add_region(vtkUnstructuredGrid& vtk_grid, const mesh::Dictionary& dict, const mesh::Region& region)
  {
    const Uint my_rank = common::PE::Comm::instance().rank();
    node_map_t& node_map = m_node_maps[std::make_pair(&dict, &region)];

    // Allocate connectivity storage
    Uint nb_cells = 0;
    for(const mesh::Entities& entities : common::find_components<mesh::Entities>(region))
    {
      if(detail::vtk_type(entities.space(dict).shape_function()) != -1)
        nb_cells += (m_include_ghost_cells ? entities.size() : std::count_if(std::begin(entities.rank().array()), std::end(entities.rank().array()), [=](const Uint element_rank){return element_rank == my_rank;}));
    }
    vtk_grid.Allocate(nb_cells);

    // Keep track of what nodes have been added
    const Uint unused_node = std::numeric_limits<Uint>::max();
    std::vector<Uint> cf3_to_vtk(dict.size(), unused_node);
    Uint vtk_point_idx = 0; // Last index of the vtk points

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
          if(cf3_to_vtk[node_idx] == unused_node)
          {
            node_map[node_idx] = vtk_point_idx;
            cf3_to_vtk[node_idx] = vtk_point_idx;
            ++vtk_point_idx;
          }
          id_list->SetId(j, cf3_to_vtk[node_idx]);
        }
        vtk_grid.InsertNextCell(detail::vtk_type(space.shape_function()), id_list);
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
  }

  std::map<key_t, node_map_t> m_node_maps;
  const bool m_include_ghost_cells;
};

CF3ToVTK::CF3ToVTK ( const std::string& name ) : common::Action ( name )
{
  options().add("mesh", m_mesh)
    .description("Mesh to convert")
    .pretty_name("Mesh")
    .link_to(&m_mesh)
    .mark_basic();

  options().add("include_ghost_elements", false)
    .pretty_name("Include ghost elements")
    .description("Include ghost elements in the target VTK mesh")
    .attach_trigger(boost::bind(&CF3ToVTK::trigger_include_ghost_elements, this))
    .mark_basic();

  trigger_include_ghost_elements();
}

CF3ToVTK::~CF3ToVTK()
{
}

void CF3ToVTK::trigger_include_ghost_elements()
{
  m_node_mapping.reset(new node_mapping(options().value<bool>("include_ghost_elements")));
}

void CF3ToVTK::execute()
{
  if(is_null(m_mesh))
    throw common::SetupError(FromHere(), "Mesh is not set for CF3ToVTK");

  const mesh::Mesh& mesh = *m_mesh;

  const bool include_ghost_elements = options().value<bool>("include_ghost_elements");
  m_multiblock_set = vtkSmartPointer<vtkMultiBlockDataSet>::New();

  const Uint nb_dicts = mesh.dictionaries().size();
  m_multiblock_set->SetNumberOfBlocks(nb_dicts);

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
      m_multiblock_set->SetBlock(dict_idx, dict_tree.top());
      m_multiblock_set->GetMetaData(dict_idx)->Set(vtkMultiBlockDataSet::NAME(), dict.name().c_str());
      nb_blocks_stack.push(0);
      detail::recurse(mesh.topology(),
      [&](const mesh::Region& region) // Region begin
      {
        const Uint nb_entities = common::find_components<mesh::Entities>(region).size();
        const Uint nb_regions = common::find_components<mesh::Region>(region).size();
        cf3_assert(!dict_tree.empty());

        vtkSmartPointer<vtkMultiBlockDataSet> parent = dict_tree.top();

        vtkSmartPointer<vtkMultiBlockDataSet> current_multiblock_set = parent;
        if(nb_regions != 0 || nb_entities == 0)
        {
          // Create a new multiblock set if there are sub-regions, or if the region is empty
          current_multiblock_set = vtkSmartPointer<vtkMultiBlockDataSet>::New();
          parent->SetBlock(nb_blocks_stack.top(), current_multiblock_set.Get());
          parent->GetMetaData(nb_blocks_stack.top()++)->Set(vtkMultiBlockDataSet::NAME(), region.name().c_str());
          current_multiblock_set->SetNumberOfBlocks(static_cast<Uint>(nb_entities != 0) + nb_regions);
          nb_blocks_stack.push(0);
        }

        dict_tree.push(current_multiblock_set); // always push, even if this was the parent (balance with pop at the end)

        vtkSmartPointer<vtkDataObject> current_data_object = current_multiblock_set;
        vtkSmartPointer<vtkUnstructuredGrid> current_grid;

        // Place elements in an unstructured grid
        if(nb_entities != 0)
        {
          current_grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
          current_multiblock_set->SetBlock(nb_blocks_stack.top(), current_grid.Get());
          current_multiblock_set->GetMetaData(nb_blocks_stack.top()++)->Set(vtkMultiBlockDataSet::NAME(), region.name().c_str());

          if(nb_regions == 0)
          {
            current_data_object = current_grid;
            nb_blocks_stack.push(0);
          }
        }

        if(is_not_null(current_grid))
        {
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


////////////////////////////////////////////////////////////////////////////////

} // namespace vtk
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////
