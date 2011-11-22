// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/replace.hpp>

#include "common/BoostFilesystem.hpp"
#include "common/Foreach.hpp"
#include "common/PE/Comm.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/StringConversion.hpp"
#include "common/Map.hpp"

#include "mesh/gmsh/Writer.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

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


  // gmsh types: http://www.geuz.org/gmsh/doc/texinfo/gmsh.html#MSH-ASCII-file-format

  m_elementTypes["cf3.mesh.LagrangeP0.Point1D"]=15;
  m_elementTypes["cf3.mesh.LagrangeP0.Point2D"]=15;
  m_elementTypes["cf3.mesh.LagrangeP0.Point3D"]=15;

  m_elementTypes["cf3.mesh.LagrangeP1.Line1D" ]=1;
  m_elementTypes["cf3.mesh.LagrangeP1.Line2D" ]=1;
  m_elementTypes["cf3.mesh.LagrangeP1.Line3D" ]=1;
  m_elementTypes["cf3.mesh.LagrangeP1.Triag2D"]=2;
  m_elementTypes["cf3.mesh.LagrangeP1.Triag3D"]=2;
  m_elementTypes["cf3.mesh.LagrangeP1.Quad2D" ]=3;
  m_elementTypes["cf3.mesh.LagrangeP1.Quad3D" ]=3;
  m_elementTypes["cf3.mesh.LagrangeP1.Tetra3D"]=4;
  m_elementTypes["cf3.mesh.LagrangeP1.Hexa3D" ]=5;

  m_elementTypes["cf3.mesh.LagrangeP2.Line1D" ]=8;
  m_elementTypes["cf3.mesh.LagrangeP2.Line2D" ]=8;
  m_elementTypes["cf3.mesh.LagrangeP2.Line3D" ]=8;
  m_elementTypes["cf3.mesh.LagrangeP2.Triag2D"]=9;
  m_elementTypes["cf3.mesh.LagrangeP2.Triag3D"]=9;
  m_elementTypes["cf3.mesh.LagrangeP2.Quad2D" ]=10;
  m_elementTypes["cf3.mesh.LagrangeP2.Quad3D" ]=10;

  m_elementTypes["cf3.mesh.LagrangeP3.Line1D" ]=26;
  m_elementTypes["cf3.mesh.LagrangeP3.Line2D" ]=26;
  m_elementTypes["cf3.mesh.LagrangeP3.Line3D" ]=26;
  m_elementTypes["cf3.mesh.LagrangeP3.Triag2D"]=21;
  m_elementTypes["cf3.mesh.LagrangeP3.Triag3D"]=21;

  m_elementTypes["cf3.mesh.LagrangeP3.Quad2D"] = 36;
  m_elementTypes["cf3.mesh.LagrangeP3.Quad3D"] = 36;

  m_cf_2_gmsh_node = create_static_component<Map<Uint,Uint> >("to_gmsh_node");

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

void Writer::write_from_to(const Mesh& mesh, const URI& file_path)
{

  m_mesh = Handle<Mesh>(mesh.handle()).get();

  // if the file is present open it
  boost::filesystem::fstream file;
  boost::filesystem::path path (file_path.path());
  if (PE::Comm::instance().size() > 1)
  {
    path = boost::filesystem::basename(path) + "_P" + to_str(PE::Comm::instance().rank()) + boost::filesystem::extension(path);
  }
//  CFLog(VERBOSE, "Opening file " <<  path.string() << "\n");
  file.open(path,std::ios_base::out);
  if (!file) // didn't open so throw exception
  {
     throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                                boost::system::error_code() );
  }

  // must be in correct order!
  write_header(file);
  m_cf_2_gmsh_node->clear();
  m_cf_2_gmsh_node->reserve(Elements::used_nodes(*m_mesh->topology().as_non_const()).size());
  write_coordinates(file);
  write_connectivity(file);
  write_elem_nodal_data(file);
  //write_nodal_data(file);
  //write_element_data(file);
  file.close();
  m_cf_2_gmsh_node->clear();
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

  // physical names
  Uint phys_name_counter(0);
  boost_foreach(const Region& groupRegion, find_components_recursively_with_filter<Region>(*m_mesh,IsGroup()))
  {
    ++phys_name_counter;
  }

  file << "$PhysicalNames\n";
  file << phys_name_counter << "\n";

  phys_name_counter=0;
  boost_foreach(const Region& groupRegion, find_components_recursively_with_filter<Region>(*m_mesh,IsGroup()))
  {
    std::string name = groupRegion.uri().path();
    boost::algorithm::replace_first(name,m_mesh->topology().uri().path()+"/","");
    m_groupnumber[groupRegion.uri().path()] = ++phys_name_counter;

    Uint group_dimensionality(0);
    boost_foreach(const Elements& elements, find_components<Elements>(groupRegion))
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

  const common::List<Uint>& used_nodes = Elements::used_nodes(*m_mesh->topology().as_non_const(),true);
  const Uint nb_nodes = used_nodes.size();
  Map<Uint,Uint>& to_gmsh_node = *m_cf_2_gmsh_node;

  file << "$Nodes\n";
  file << nb_nodes << "\n";

  Uint node_number=0;
  const common::Table<Real>& coordinates = m_mesh->geometry_fields().coordinates();
  Uint gmsh_node = 1;
  boost_foreach( const Uint node, used_nodes.array())
  {
    to_gmsh_node.insert_blindly(node,gmsh_node++);
    common::Table<Real>::ConstRow coord = coordinates[node];
    file << ++node_number << " ";
    for (Uint d=0; d<3; d++)
    {
      if (d<m_mesh->dimension())
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

  // file << "$Elements                                                               \n";
  // file << "number-of-elements                                                      \n";
  // file << "elm-number elm-type number-of-tags < tag > ... node-number-list ...     \n";
  // file << "$EndElements\n";
  Uint nbElems = m_mesh->topology().recursive_elements_count();
  Map<Uint,Uint>& to_gmsh_node = *m_cf_2_gmsh_node;

  file << "$Elements\n";
  file << nbElems << "\n";
  std::string group_name("");
  Uint group_number;
  Uint elm_type;
  Uint number_of_tags=3; // 1 for physical entity,  1 for elementary geometrical entity,  1 for mesh partition
  Uint elm_number=0;
  Uint partition_number = PE::Comm::instance().rank();

  boost_foreach(const Entities& elements, m_mesh->topology().elements_range())
  {
    group_name = elements.parent().uri().path();
    group_number = m_groupnumber[group_name];

    m_element_start_idx[&elements]=elm_number;

    //file << "// Region " << elements.uri().string() << "\n";
    elm_type = m_elementTypes[elements.element_type().derived_type_name()];
    const Uint nb_elem = elements.size();
    for (Uint e=0; e<nb_elem; ++e, ++elm_number)
    {
      file << elm_number+1 << " " << elm_type << " " << number_of_tags << " " << group_number << " " << 0 << " " << partition_number;
      boost_foreach(const Uint node_idx, elements.get_nodes(e))
      {
        file << " " << to_gmsh_node[node_idx];
      }
      file << "\n";
    }
  }
  file << "$EndElements\n";
}

//////////////////////////////////////////////////////////////////////


void Writer::write_elem_nodal_data(std::fstream& file)
{
//  $ElementNodeData
//  number-of-string-tags
//  < "string-tag" >
//  ...
//  number-of-real-tags
//  < real-tag >
//  ...
//  number-of-integer-tags
//  < integer-tag >
//  ...
//  elm-number number-of-nodes-per-element value ...
//  ...
//  $ElementEndNodeData

  // set precision for Real
  Uint prec = file.precision();
  file.precision(8);

  boost_foreach(Handle<Field> field_ptr, m_fields)
  {
    cf3_assert(is_null(field_ptr) == false);
    Field& field = *field_ptr;
//    if (field.basis() == SpaceFields::Basis::ELEMENT_BASED ||
//        field.basis() == SpaceFields::Basis::CELL_BASED    ||
//        field.basis() == SpaceFields::Basis::FACE_BASED    )
    {
      const Real field_time = 0;//field.option("time").value<Real>();
      const Uint field_iter = 0;//field.option("iteration").value<Uint>();
      const std::string field_name = field.name();
      std::string field_topology = field.topology().uri().path();
      const std::string field_basis = SpaceFields::Basis::Convert::instance().to_str(field.basis());
      boost::algorithm::replace_first(field_topology,m_mesh->topology().uri().path(),"");
      Uint nb_elements = 0;
      boost_foreach(Entities& elements, find_components_recursively<Entities>(field.topology()))
      {
        if (field.elements_lookup().contains(elements))
        {
          nb_elements += elements.size();
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

        file << "$ElementNodeData\n";
        file << 4 << "\n";
        file << "\"" << (var_name == "var" ? field_name+to_str(iVar) : var_name) << "\"\n";
        file << "\"" << field_name << "\"\n";
        file << "\"" << field_topology << "\"\n";
        file << "\"" << field_basis << "\"\n";
        file << 1 << "\n" << field_time << "\n";
        file << 3 << "\n" << field_iter << "\n" << datasize << "\n" << nb_elements <<"\n";

        boost_foreach(Entities& elements, find_components_recursively<Entities>(field.topology()))
        {
          if (field.elements_lookup().contains(elements))
          {
            Space& field_space = field.space(elements);
            Uint elm_number = m_element_start_idx[&elements];
            Uint local_nb_elms = elements.size();

            const Uint nb_states = field_space.nb_states();
            RealMatrix field_data (nb_states,var_type);

            const Uint nb_nodes = elements.element_type().nb_nodes();

            /// write element
            for (Uint local_elm_idx = 0; local_elm_idx<local_nb_elms; ++local_elm_idx)
            {
              file << ++elm_number << " " << nb_nodes << " ";

              /// set field data
              Connectivity::ConstRow field_indexes = field_space.indexes_for_element(local_elm_idx);
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

  //  $ElementNodeData
  //  number-of-string-tags
  //  < "string-tag" >
  //  ...
  //  number-of-real-tags
  //  < real-tag >
  //  ...
  //  number-of-integer-tags
  //  < integer-tag >
  //  ...
  //  elm-number number-of-nodes-per-element value ...
  //  ...
  //  $ElementEndNodeData

  // set precision for Real
  Uint prec = file.precision();
  file.precision(8);

  boost_foreach(Handle<Field> field_ptr, m_fields)
  {
    Field& field = *field_ptr;

    if (field.basis() == SpaceFields::Basis::POINT_BASED)
    {
      const std::string field_name = field.name();
      std::string field_topology = field.topology().uri().path();
      boost::algorithm::replace_first(field_topology,m_mesh->topology().uri().path(),"");
      const Real field_time = field.option("time").value<Real>();
      const Uint field_iter = field.option("iteration").value<Uint>();
      // data_header
      Uint row_idx=0;
      for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
      {
        Field::VarType var_type = field.var_type(iVar);
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

        RealVector data(datasize);
        Uint nb_nodes = field.size();

        file << "$NodeData\n";
        file << 3 << "\n";
        file << "\"" << (var_name == "var" ? field_name+to_str(iVar) : var_name) << "\"\n";
        file << "\"" << field_name << "\"\n";
        file << "\"" << field_topology << "\"\n";
        file << 1 << "\n" << field_time << "\n";
        file << 3 << "\n" << field_iter << "\n" << datasize << "\n" << nb_nodes <<"\n";


        Uint local_node_idx=0;
        const Map<Uint,Uint>& to_gmsh_node = *m_cf_2_gmsh_node;

        boost_foreach(Field::ConstRow field_per_node, field.array())
        {
          file << to_gmsh_node[used_nodes[local_node_idx++]] << " ";

          if (var_type==Field::TENSOR_2D)
          {
            data[0]=field_per_node[row_idx+0];
            data[1]=field_per_node[row_idx+1];
            data[3]=field_per_node[row_idx+2];
            data[4]=field_per_node[row_idx+3];
            for (Uint idx=0; idx<datasize; ++idx)
              file << " " << data[idx];
          }
          else
          {
            for (Uint idx=row_idx; idx<row_idx+Uint(var_type); ++idx)
              file << " " << field_per_node[idx];
            if (var_type == Field::VECTOR_2D)
              file << " " << 0.0;
          }
          file << "\n";
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

  //  $ElementNodeData
  //  number-of-string-tags
  //  < "string-tag" >
  //  ...
  //  number-of-real-tags
  //  < real-tag >
  //  ...
  //  number-of-integer-tags
  //  < integer-tag >
  //  ...
  //  elm-number number-of-nodes-per-element value ...
  //  ...
  //  $ElementEndNodeData

  // set precision for Real
  Uint prec = file.precision();
  file.precision(8);

  boost_foreach(Handle<Field> field_ptr, m_fields)
  {
    Field& field = *field_ptr;
    if (field.basis() == SpaceFields::Basis::ELEMENT_BASED ||
        field.basis() == SpaceFields::Basis::CELL_BASED    ||
        field.basis() == SpaceFields::Basis::FACE_BASED    )
    {
      const Real field_time = field.option("time").value<Real>();
      const Uint field_iter = field.option("iteration").value<Uint>();
      const std::string field_name = field.name();
      std::string field_topology = field.topology().uri().path();
      const std::string field_basis = SpaceFields::Basis::Convert::instance().to_str(field.basis());
      boost::algorithm::replace_first(field_topology,m_mesh->topology().uri().path(),"");
      Uint nb_elements = 0;
      boost_foreach(Entities& field_elements, find_components_recursively<Entities>(field.topology()))
      {
        if (field.elements_lookup().contains(field_elements))
        {
          nb_elements += field_elements.size();
        }
      }

      // data_header
      Uint row_idx=0;
      for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
      {
        Field::VarType var_type = field.var_type(iVar);
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
        RealVector data(datasize);
        data.setZero();

        file << "$ElementData\n";
        file << 4 << "\n";
        file << "\"" << (var_name == "var" ? field_name+to_str(iVar) : var_name) << "\"\n";
        file << "\"" << field_name << "\"\n";
        file << "\"" << field_topology << "\"\n";
        file << "\"" << field_basis << "\"\n";
        file << 1 << "\n" << field_time << "\n";
        file << 3 << "\n" << field_iter << "\n" << datasize << "\n" << nb_elements <<"\n";
        boost_foreach(Entities& field_elements, find_components_recursively<Entities>(field.topology()))
        {
          if (field.elements_lookup().contains(field_elements))
          {
            Space& field_space = field.space(field_elements);
            Uint elm_number = m_element_start_idx[&field_elements];
            Uint local_nb_elms = field_elements.size();
            for (Uint local_elm_idx = 0; local_elm_idx<local_nb_elms; ++local_elm_idx)
            {
              Uint field_index = field_space.indexes_for_element(local_elm_idx)[0];
              file << ++elm_number << " " ;
              if (var_type==Field::TENSOR_2D)
              {
                data[0]=field[field_index][row_idx+0];
                data[1]=field[field_index][row_idx+1];
                data[3]=field[field_index][row_idx+2];
                data[4]=field[field_index][row_idx+3];
                for (Uint idx=0; idx<datasize; ++idx)
                  file << " " << data[idx];
              }
              else
              {
                for (Uint idx=row_idx; idx<row_idx+Uint(var_type); ++idx)
                  file << " " << field[field_idx][idx];
                if (var_type == Field::VECTOR_2D)
                  file << " " << 0.0;
              }
              file << "\n";
            }
          }
        }
        file << "$EndElementData\n";
        row_idx += Uint(var_type);
      }
    }

  }
  // restore precision
  file.precision(prec);
}
*/
//////////////////////////////////////////////////////////////////////////////

} // gmsh
} // mesh
} // cf3
