// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/replace.hpp>
#include <boost/thread/thread.hpp>

#include "common/Log.hpp"
#include "common/BoostFilesystem.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/Foreach.hpp"
#include "common/PE/Comm.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/StringConversion.hpp"
#include "common/Map.hpp"

#include "mesh/gmsh/Writer.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshMetadata.hpp"
#include "mesh/Region.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Functions.hpp"


//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;

namespace cf3 {
namespace mesh {
namespace gmsh {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < gmsh::Writer, MeshWriter, LibGmsh> aGmshWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

Writer::Writer( const std::string& name )
: MeshWriter(name)
{
  options().add("serial",false)
      .pretty_name("Serial Format")
      .description("All processors write in 1 file")
      .mark_basic();

  // gmsh types: http://www.geuz.org/gmsh/doc/texinfo/gmsh.html#MSH-ASCII-file-format

  m_elementTypes["cf3.mesh.LagrangeP0.Point1D"]=P0POINT;
  m_elementTypes["cf3.mesh.LagrangeP0.Point2D"]=P0POINT;
  m_elementTypes["cf3.mesh.LagrangeP0.Point3D"]=P0POINT;

  m_elementTypes["cf3.mesh.LagrangeP1.Line1D" ]=P1LINE;
  m_elementTypes["cf3.mesh.LagrangeP1.Line2D" ]=P1LINE;
  m_elementTypes["cf3.mesh.LagrangeP1.Line3D" ]=P1LINE;
  m_elementTypes["cf3.mesh.LagrangeP1.Triag2D"]=P1TRIAG;
  m_elementTypes["cf3.mesh.LagrangeP1.Triag3D"]=P1TRIAG;
  m_elementTypes["cf3.mesh.LagrangeP1.Quad2D" ]=P1QUAD;
  m_elementTypes["cf3.mesh.LagrangeP1.Quad3D" ]=P1QUAD;
  m_elementTypes["cf3.mesh.LagrangeP1.Tetra3D"]=P1TETRA;
  m_elementTypes["cf3.mesh.LagrangeP1.Hexa3D" ]=P1HEXA;

  m_elementTypes["cf3.mesh.LagrangeP2.Line1D" ]=P2LINE;
  m_elementTypes["cf3.mesh.LagrangeP2.Line2D" ]=P2LINE;
  m_elementTypes["cf3.mesh.LagrangeP2.Line3D" ]=P2LINE;
  m_elementTypes["cf3.mesh.LagrangeP2.Triag2D"]=P2TRIAG;
  m_elementTypes["cf3.mesh.LagrangeP2.Triag3D"]=P2TRIAG;
  m_elementTypes["cf3.mesh.LagrangeP2.Quad2D" ]=P2QUAD;
  m_elementTypes["cf3.mesh.LagrangeP2.Quad3D" ]=P2QUAD;

  m_elementTypes["cf3.mesh.LagrangeP3.Line1D" ]=P3LINE;
  m_elementTypes["cf3.mesh.LagrangeP3.Line2D" ]=P3LINE;
  m_elementTypes["cf3.mesh.LagrangeP3.Line3D" ]=P3LINE;
  m_elementTypes["cf3.mesh.LagrangeP3.Triag2D"]=P3TRIAG;
  m_elementTypes["cf3.mesh.LagrangeP3.Triag3D"]=P3TRIAG;
  m_elementTypes["cf3.mesh.LagrangeP3.Quad2D" ]=P3QUAD;
  m_elementTypes["cf3.mesh.LagrangeP3.Quad3D" ]=P3QUAD;

}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Writer::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".msh");
  extensions.push_back(".gmsh");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void Writer::write()
{
  // if the file is present open it
  boost::filesystem::fstream file;
  boost::filesystem::path path (m_file_path.path());
  path = path.parent_path() / boost::filesystem::path (boost::filesystem::basename(path) + "_P" + to_str(PE::Comm::instance().rank()) + boost::filesystem::extension(path));
  file.open(path,std::ios_base::out);
  if (!file) // didn't open so throw exception
  {
     throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                                boost::system::error_code() );
  }

  write_header(file);
  write_coordinates(file);
  write_connectivity(file);
  write_elem_nodal_data(file);
  write_nodal_data(file);
  file.close();

  // Write post-processing file, merging all parallel files
  if (PE::Comm::instance().rank() == 0)
  {
    boost::filesystem::fstream parallel_file;
    boost::filesystem::path parallel_file_path (m_file_path.path());
    parallel_file.open(parallel_file_path,std::ios_base::out);
    if (!parallel_file) // didn't open so throw exception
    {
       throw boost::filesystem::filesystem_error( parallel_file_path.string() + " failed to open",
                                                  boost::system::error_code() );
    }

    for (Uint r=0; r<PE::Comm::instance().size(); ++r)
    {
      boost::filesystem::path rank_file_path (m_file_path.path());
      rank_file_path = boost::filesystem::basename(rank_file_path) + "_P" + to_str(r) + boost::filesystem::extension(rank_file_path);
      parallel_file << "Merge \"" << rank_file_path.string() << "\";" << std::endl;
    }
    parallel_file.close();
  }

}
/////////////////////////////////////////////////////////////////////////////

void Writer::write_header(std::fstream& file)
{
  std::string version = "2";
  Uint file_type = 0; // ASCII
  Uint data_size = 8; // double precision

  // format
  file << "$MeshFormat\n";
  file << version << " " << file_type << " " << data_size << "\n";
  file << "$EndMeshFormat\n";

  m_groupnumber.clear();

  // Count the number of physical groups
  Uint phys_name_counter(0);
  std::vector< Handle<Region const> > phys_group_regions;
  cf3_assert(m_regions.size());
  boost_foreach(const Handle<Region const>& region, m_regions)
  {
    // Just in case this region itself contains entities
    if (m_region_filter(*region))
    {
      ++phys_name_counter;
      phys_group_regions.push_back(region);
    }
    else // look inside this region for regions containing entities
    {
      boost_foreach(const Region& phys_group_region, find_components_recursively_with_filter<Region>(*region,m_region_filter))
      {
        ++phys_name_counter;
        phys_group_regions.push_back(phys_group_region.handle<Region>());
      }
    }
  }

  file << "$PhysicalNames\n";
  file << phys_name_counter << "\n";

  phys_name_counter=0;
  boost_foreach(const Handle<Region const>& phys_group_region, phys_group_regions)
  {
    std::string name = phys_group_region->uri().path();
    boost::algorithm::replace_first(name,m_mesh->topology().uri().path()+"/","");
    m_groupnumber[phys_group_region->uri().path()] = ++phys_name_counter;

    Uint group_dimensionality(0);
    boost_foreach(const Elements& elements, find_components_with_filter<Elements>(*phys_group_region,m_entities_filter))
      group_dimensionality = std::max(group_dimensionality, elements.element_type().dimensionality());

    file << group_dimensionality << " " << phys_name_counter << " \"" << name << "\"\n";
  }
  file << "$EndPhysicalNames\n";
}

//////////////////////////////////////////////////////////////////////////////

void Writer::write_coordinates(std::fstream& file)
{
  // set precision for Real
  Uint prec = file.precision();
  file.precision(8);

  // Assemble a list of all the coordinates that are used in this mesh
  const boost::shared_ptr< common::List<Uint> > used_nodes_ptr = build_used_nodes_list(m_filtered_entities,m_mesh->geometry_fields(),m_enable_overlap);
  const common::List<Uint>& used_nodes = *used_nodes_ptr;

  // Create a mapping between the actual node-numbering in the mesh, and the node-numbering to be written
  const Uint nb_nodes = used_nodes.size();

  file << "$Nodes\n";
  file << nb_nodes << "\n";

  const Uint nb_dim = m_mesh->dimension();
  Uint node_number=0;
  const Dictionary& geometry = m_mesh->geometry_fields();
  const common::Table<Real>& coordinates = geometry.coordinates();
  Uint gmsh_node = 1;
  boost_foreach( const Uint node, used_nodes.array())
  {
    common::Table<Real>::ConstRow coord = coordinates[node];
    file << geometry.glb_idx()[node]+1 << " ";
    for (Uint d=0; d<3; d++)
    {
      if (d<nb_dim)
        file << coord[d] << " ";
      else
        file << 0 << " ";
    }
    file << "\n";
  }

  file << "$EndNodes\n";
  // restore precision
  file.precision(prec);
}

//////////////////////////////////////////////////////////////////////////////

void Writer::write_connectivity(std::fstream& file)
{
  /// Elements section:
  /// @code
  /// $Elements
  /// number-of-elements
  /// elem-number   elem-type   number-of-tags(3)  tag1(group_number)  tag2(elementary_entity_index)  tag3(partition)  elem-node-list
  /// $EndElements
  /// @endcode
  /// @note partition number (tag3) is set to -1 for ghost elements (conforming Gmsh standard format)

  Uint nb_elems = 0;
  boost_foreach(const Handle<Region const>& region, m_regions)
      nb_elems += region->recursive_filtered_elements_count(m_entities_filter,m_enable_overlap);

  file << "$Elements\n";
  file << nb_elems << "\n";
  std::string group_name("");
  Uint group_number;
  Uint elm_type;
  Uint number_of_tags=3; // 1 for physical entity,  1 for elementary geometrical entity,  1 for mesh partition
  Uint partition_number = PE::Comm::instance().rank();

  Uint elementary_entity_index=1;
  boost_foreach(const Handle<Entities const>& elements, m_filtered_entities)
  {
    group_name = elements->parent()->uri().path();
    group_number = m_groupnumber[group_name];
    elm_type = m_elementTypes[elements->element_type().derived_type_name()];
    const Connectivity& element_connectivity = elements->geometry_space().connectivity();
    const Uint nb_elem = elements->size();
    bool ghost;
    for (Uint e=0; e<nb_elem; ++e)
    {
      ghost = elements->is_ghost(e);
      if( m_enable_overlap || !ghost )
      {
        file << elements->glb_idx()[e]+1 << " " << elm_type << " " << number_of_tags << " " << group_number << " " << elementary_entity_index << " " << (ghost? -1 : partition_number);
        boost_foreach(const Uint node_idx, element_connectivity[e])
        {
          file << " " << elements->geometry_fields().glb_idx()[node_idx]+1;
        }
        file << "\n";
      }
    }
    ++elementary_entity_index;
  }
  file << "$EndElements\n";
}

//////////////////////////////////////////////////////////////////////


void Writer::write_elem_nodal_data(std::fstream& file)
{

  /// Discontinuous fields section
  /// @code
  /// $ElementNodeData
  ///  number-of-string-tags
  ///  < "string-tag" >
  ///  ...
  ///  number-of-real-tags
  ///  < real-tag >
  ///  ...
  ///  number-of-integer-tags
  ///  < integer-tag >
  ///  ...
  ///  elm-number number-of-nodes-per-element value ...
  ///  ...
  ///  $ElementEndNodeData
  /// @endcode

  // set precision for Real
  Uint prec = file.precision();
  file.precision(8);

  boost_foreach(Handle<Field const> field_h, m_fields)
  {
    cf3_assert(is_null(field_h) == false);
    const Field& field = *field_h;
    if(field.discontinuous())
    {
      const Real field_time = m_mesh->metadata().properties().value<Real>("time");
      const Uint field_iter = m_mesh->metadata().properties().value<Uint>("iter");
      const std::string field_name = field.name();
      Uint nb_elements = 0;
      boost_foreach(const Handle<Entities const>& elements_handle, m_filtered_entities )
      {
        if (field.dict().defined_for_entities(elements_handle))
        {
          nb_elements += elements_handle->size();
          if (m_enable_overlap==false)
          {
            Uint nb_ghost=0;
            for(Uint e=0; e<elements_handle->size(); ++e)
            {
              if (elements_handle->is_ghost(e))
                ++nb_ghost;
            }
            nb_elements -= nb_ghost;
          }
        }
      }
      // data_header
      Uint row_idx=0;
      for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
      {
        Field::VarType var_type = field.var_length(iVar);
        std::string var_name = field.var_name(iVar);

        Uint datasize(var_type);
        switch (var_type)
        {
          case Field::VECTOR_2D:
            datasize=Uint(Field::VECTOR_3D);
            break;
          case Field::TENSOR_2D:
            datasize=Uint(Field::TENSOR_3D);
            break;
          default:
            break;
        }
        RealVector data(datasize); data.setZero();

        CFdebug << "Writing discontinuous field " << field.uri() << " with " << nb_elements << " elements" << CFendl;

        file << "$ElementNodeData\n";

        // add 2 string tags : var_name, field_name
        file << 2 << "\n";
        file << "\"" << (var_name == "var" ? field_name+to_str(iVar) : var_name) << "\"\n";
        file << "\"" << field_name << "\"\n";
        // add 1 real tag: time
        file << 1 << "\n" << field_time << "\n";  // 1 real tag: time
        // add 3 integer tags: time_step, variable_type, nb elements
        file << 3 << "\n" << field_iter << "\n" << datasize << "\n" << nb_elements <<"\n";

        boost_foreach(const Handle<Entities const>& elements_handle, m_filtered_entities )
        {
          if (field.dict().defined_for_entities(elements_handle))
          {
            const Entities& elements = *elements_handle;
            const Space& field_space = field.space(elements);
            Uint local_nb_elms = elements.size();

            const Uint nb_states = field_space.shape_function().nb_nodes();
            RealMatrix field_data (nb_states,static_cast<int>(var_type));

            const Uint nb_nodes = elements.element_type().nb_nodes();

            /// write element
            for (Uint local_elm_idx = 0; local_elm_idx<local_nb_elms; ++local_elm_idx)
            {
              if (m_enable_overlap || !elements.is_ghost(local_elm_idx))
              {
                file << elements.glb_idx()[local_elm_idx]+1 << " " << nb_nodes << " ";
                /// set field data
                Connectivity::ConstRow field_indexes = field_space.connectivity()[local_elm_idx];
                for (Uint iState=0; iState<nb_states; ++iState)
                {
                  for (Uint j=0; j<var_type; ++j)
                    field_data(iState,j) = field[field_indexes[iState]][row_idx+j];
                }

                for (Uint iNode=0; iNode<nb_nodes; ++iNode)
                {
                  /// get element_node local coordinates
                  RealVector local_coords = elements.element_type().shape_function().local_coordinates().row(iNode);

                  /// evaluate field shape function in element_node
                  RealVector node_data = field_space.shape_function().value(local_coords)*field_data;
                  cf3_assert(node_data.size() == var_type);

                  if (var_type==Field::TENSOR_2D)
                  {
                    data[0]=node_data[0];
                    data[1]=node_data[1];
                    data[3]=node_data[2];
                    data[4]=node_data[3];
                    for (Uint idx=0; idx<datasize; ++idx)
                      file << " " << data[idx];
                  }
                  else
                  {
                    for (Uint j=0; j<var_type; ++j)
                      file << " " << node_data[j];
                    if (var_type == Field::VECTOR_2D)
                      file << " " << 0.0;
                  }
                }
                file << "\n";
              }
            }
          }
        }
        file << "$EndElementNodeData\n";
        row_idx += Uint(var_type);
      }
    }
  }
  // restore precision
  file.precision(prec);
}


////////////////////////////////////////////////////////////////////////////////

/*
 * Algorithm:
 * 1) assemble geometry used_nodes list for each field
 * 2) loop elements and interpolate to geometry nodes
 * 3) when a geometry-node is interpolated, add it to a list_of_interpolated_geom_nodes
 * 4) Nodes can be written to file in any order, as long as the geometry::glb_idx is used as the node-index.
 */
void Writer::write_nodal_data(std::fstream& file)
{

  //  $NodeData
  //  1              // 1 string tag:
  //  "a_string_tag" //   the name of the view
  //  1              // 1 real tag:
  //  0              //  time value == 0
  //  3              // 3 integer tags:
  //  0              //  time step == 0 (time steps always start at 0)
  //  1              //  1-component field (scalar) (only 1,3,9 are valid)
  //  6              //  6 values follow:
  //  1 0.0          //  value associated with node 1 == 0.0
  //  2 0.1          //  ...
  //  3 0.2
  //  4 0.0
  //  5 0.2
  //  6 0.4
  //  $EndNodeData


  // set precision for Real
  Uint prec = file.precision();
  file.precision(8);


  boost_foreach(Handle<Field const> field_h, m_fields)
  {
    cf3_assert(is_null(field_h) == false);
    const Field& field = *field_h;
    if(field.continuous())
    {
      cf3_assert(is_null(field_h) == false);
      const Field& field = *field_h;
      const Real field_time = m_mesh->metadata().properties().value<Real>("time");
      const Uint field_iter = m_mesh->metadata().properties().value<Uint>("iter");
      const std::string field_name = field.name();
      Uint nb_elements = 0;
      std::vector< Handle<Entities const> > filtered_used_entities_by_field;
      boost_foreach(const Handle<Entities const>& elements_handle, m_filtered_entities )
      {
        if (field.dict().defined_for_entities(elements_handle))
        {
          filtered_used_entities_by_field.push_back(elements_handle);
        }
      }

      const boost::shared_ptr< common::List<Uint> > used_nodes_ptr = build_used_nodes_list(filtered_used_entities_by_field,m_mesh->geometry_fields(),m_enable_overlap);
      const common::List<Uint>& used_nodes = *used_nodes_ptr;
      std::vector<bool> is_node_visited(m_mesh->geometry_fields().size(),false);

      // data_header
      Uint row_idx=0;
      for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
      {
        is_node_visited.assign(m_mesh->geometry_fields().size(),false);
        Field::VarType var_type = field.var_length(iVar);
        std::string var_name = field.var_name(iVar);

        Uint datasize(var_type);
        switch (var_type)
        {
          case Field::VECTOR_2D:
            datasize=Uint(Field::VECTOR_3D);
            break;
          case Field::TENSOR_2D:
            datasize=Uint(Field::TENSOR_3D);
            break;
          default:
            break;
        }
        RealVector data(datasize); data.setZero();

        CFdebug << "Writing continuous field " << field.uri() << " with " << field.size() << " nodes" << CFendl;

        file << "$NodeData\n";

        // add 2 string tags : var_name, field_name
        file << 2 << "\n";
        file << "\"" << (var_name == "var" ? field_name+to_str(iVar) : var_name) << "\"\n";
        file << "\"" << field_name << "\"\n";
        // add 1 real tag: time
        file << 1 << "\n" << field_time << "\n";  // 1 real tag: time
        // add 3 integer tags: time_step, variable_type, nb elements
        file << 3 << "\n" << field_iter << "\n" << datasize << "\n" << used_nodes.size() <<"\n";

        boost_foreach (const Handle<Entities const>& elements_handle, filtered_used_entities_by_field)
        {
          const Space& field_space = elements_handle->space(field.dict());
          const Space& geom_space  = elements_handle->geometry_space();
          const Uint nb_elems = elements_handle->size();

          const Uint nb_states = field_space.shape_function().nb_nodes();
          RealMatrix field_data (nb_states, static_cast<int>(var_type));

          for (Uint elem_idx=0; elem_idx<nb_elems; ++elem_idx)
          {
            if (m_enable_overlap || !elements_handle->is_ghost(elem_idx))
            {
              Connectivity::ConstRow field_space_nodes = field_space.connectivity()[elem_idx];
              Connectivity::ConstRow geom_space_nodes = field_space.support().geometry_space().connectivity()[elem_idx];

              /// set field data
              Connectivity::ConstRow field_indexes = field_space.connectivity()[elem_idx];
              for (Uint iState=0; iState<nb_states; ++iState)
              {
                for (Uint j=0; j<var_type; ++j)
                  field_data(iState,j) = field[field_indexes[iState]][row_idx+j];
              }

              for (Uint elem_node_idx=0; elem_node_idx<geom_space_nodes.size(); ++elem_node_idx)
              {
                const Uint geom_space_node = geom_space_nodes[elem_node_idx];
                cf3_assert(geom_space_node < is_node_visited.size());
                if (!is_node_visited[geom_space_node])
                {
                  is_node_visited[geom_space_node]=true;

                  // * interpolate
                  /// get element_node local coordinates
                  RealVector local_coords = geom_space.shape_function().local_coordinates().row(elem_node_idx);

                  /// evaluate field shape function in element_node

                  RealVector node_data = field_space.shape_function().value(local_coords)*field_data;
                  cf3_assert(node_data.size() == var_type);

                  // * write
                  file << m_mesh->geometry_fields().glb_idx()[geom_space_node]+1 << " ";

                  if (var_type==Field::TENSOR_2D)
                  {
                    data[0]=node_data[0];
                    data[1]=node_data[1];
                    data[3]=node_data[2];
                    data[4]=node_data[3];
                    for (Uint idx=0; idx<datasize; ++idx)
                      file << " " << data[idx];
                  }
                  else
                  {
                    for (Uint j=0; j<var_type; ++j)
                      file << " " << node_data[j];
                    if (var_type == Field::VECTOR_2D)
                      file << " " << 0.0;
                  }
                  file << "\n";
                }
              }
            }
          }
        }
        file << "$EndNodeData\n";
        row_idx += Uint(var_type);
      }
    }
  }
  // restore precision
  file.precision(prec);
}

////////////////////////////////////////////////////////////////////////////////
/*
// Could be useful for visualizing extra element information such as global index, rank,
// The only advantage over write_element_nodal_data() is that it is taking less file memory.
void Writer::write_element_data(std::fstream& file)
{
  //  $ElementData
  //  1              // 1 string tag:
  //  "a_string_tag" //   the name of the view
  //  1              // 1 real tag:
  //  0              //  time value == 0
  //  3              // 3 integer tags:
  //  0              //  time step == 0 (time steps always start at 0)
  //  1              //  1-component field (scalar) (only 1,3,9 are valid)
  //  6              //  data size: 6 values follow:
  //  1 0.0          //  value associated with element 1 == 0.0
  //  2 0.1          //  ...
  //  3 0.2
  //  4 0.0
  //  5 0.2
  //  6 0.4
  //  $EndElementData
}
*/
//////////////////////////////////////////////////////////////////////////////

} // gmsh
} // mesh
} // cf3
