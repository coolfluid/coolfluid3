// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/replace.hpp>

#include "Common/BoostFilesystem.hpp"
#include "Common/Foreach.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/StringConversion.hpp"

/// @todo remove
#include "Common/Log.hpp"

#include "Mesh/Gmsh/CWriter.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CFieldView.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Mesh {
namespace Gmsh {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Gmsh::CWriter, CMeshWriter, LibGmsh> aGmshWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const std::string& name )
: CMeshWriter(name)
{


  // gmsh types: http://www.geuz.org/gmsh/doc/texinfo/gmsh.html#MSH-ASCII-file-format

  m_elementTypes["CF.Mesh.SF.Point1DLagrangeP0"]=15;
  m_elementTypes["CF.Mesh.SF.Point2DLagrangeP0"]=15;
  m_elementTypes["CF.Mesh.SF.Point3DLagrangeP0"]=15;

  m_elementTypes["CF.Mesh.SF.Line1DLagrangeP1" ]=1;
  m_elementTypes["CF.Mesh.SF.Line2DLagrangeP1" ]=1;
  m_elementTypes["CF.Mesh.SF.Line3DLagrangeP1" ]=1;
  m_elementTypes["CF.Mesh.SF.Triag2DLagrangeP1"]=2;
  m_elementTypes["CF.Mesh.SF.Triag3DLagrangeP1"]=2;
  m_elementTypes["CF.Mesh.SF.Quad2DLagrangeP1" ]=3;
  m_elementTypes["CF.Mesh.SF.Quad3DLagrangeP1" ]=3;
  m_elementTypes["CF.Mesh.SF.Tetra3DLagrangeP1"]=4;
  m_elementTypes["CF.Mesh.SF.Hexa3DLagrangeP1" ]=5;

  m_elementTypes["CF.Mesh.SF.Line1DLagrangeP2" ]=8;
  m_elementTypes["CF.Mesh.SF.Line2DLagrangeP2" ]=8;
  m_elementTypes["CF.Mesh.SF.Line3DLagrangeP2" ]=8;
  m_elementTypes["CF.Mesh.SF.Triag2DLagrangeP2"]=9;
  m_elementTypes["CF.Mesh.SF.Triag3DLagrangeP2"]=9;
  m_elementTypes["CF.Mesh.SF.Quad2DLagrangeP2" ]=10;
  m_elementTypes["CF.Mesh.SF.Quad3DLagrangeP2" ]=10;

  m_elementTypes["CF.Mesh.SF.Line1DLagrangeP3" ]=26;
  m_elementTypes["CF.Mesh.SF.Line2DLagrangeP3" ]=26;
  m_elementTypes["CF.Mesh.SF.Line3DLagrangeP3" ]=26;
  m_elementTypes["CF.Mesh.SF.Triag2DLagrangeP3"]=21;
  m_elementTypes["CF.Mesh.SF.Triag3DLagrangeP3"]=21;

  m_elementTypes["CF.Mesh.SF.Quad2DLagrangeP3"] = 36;
  m_elementTypes["CF.Mesh.SF.Quad3DLagrangeP3"] = 36;
}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CWriter::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".msh");
  extensions.push_back(".gmsh");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_from_to(const CMesh& mesh, const URI& file_path)
{

  m_mesh = mesh.as_ptr<CMesh>();

  // if the file is present open it
  boost::filesystem::fstream file;
  boost::filesystem::path path (file_path.path());
  if (mpi::PE::instance().size() > 1)
  {
    path = boost::filesystem::basename(path) + "_P" + to_str(mpi::PE::instance().rank()) + boost::filesystem::extension(path);
  }
//  CFLog(VERBOSE, "Opening file " <<  path.string() << "\n");
  file.open(path,std::ios_base::out);
  if (!file) // didn't open so throw exception
  {
     throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                                boost::system::error_code() );
  }

  compute_mesh_specifics();

  // must be in correct order!
  write_header(file);
  write_coordinates(file);
  write_connectivity(file);
  write_elem_nodal_data(file);
  write_nodal_data(file);
  //write_element_data(file);
  file.close();

}
/////////////////////////////////////////////////////////////////////////////

void CWriter::write_header(std::fstream& file)
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
  boost_foreach(const CRegion& groupRegion, find_components_recursively_with_filter<CRegion>(*m_mesh,IsGroup()))
  {
    ++phys_name_counter;
  }

  file << "$PhysicalNames\n";
  file << phys_name_counter << "\n";

  phys_name_counter=0;
  boost_foreach(const CRegion& groupRegion, find_components_recursively_with_filter<CRegion>(*m_mesh,IsGroup()))
  {
    std::string name = groupRegion.full_path().path();
    boost::algorithm::replace_first(name,m_mesh->topology().full_path().path()+"/","");
    m_groupnumber[groupRegion.full_path().path()] = ++phys_name_counter;

    Uint group_dimensionality(0);
    boost_foreach(const CElements& elements, find_components<CElements>(groupRegion))
      group_dimensionality = std::max(group_dimensionality, elements.element_type().dimensionality());

    file << group_dimensionality << " " << phys_name_counter << " \"" << name << "\"\n";

  }
  file << "$EndPhysicalNames\n";
}

//////////////////////////////////////////////////////////////////////////////

void CWriter::write_coordinates(std::fstream& file)
{
  // set precision for Real
  Uint prec = file.precision();
  file.precision(8);

  const Uint nb_nodes = m_mesh->nodes().size();

  file << "$Nodes\n";
  file << nb_nodes << "\n";

  Uint node_number=0;
  boost_foreach(CTable<Real>::ConstRow row, m_mesh->nodes().coordinates().array())
  {
    file << ++node_number << " ";
    for (Uint d=0; d<3; d++)
    {
      if (d<m_coord_dim)
        file << row[d] << " ";
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

void CWriter::write_connectivity(std::fstream& file)
{

  // file << "$Elements                                                               \n";
  // file << "number-of-elements                                                      \n";
  // file << "elm-number elm-type number-of-tags < tag > ... node-number-list ...     \n";
  // file << "$EndElements\n";
  Uint nbElems = m_mesh->topology().recursive_elements_count();

  file << "$Elements\n";
  file << nbElems << "\n";
  std::string group_name("");
  Uint group_number;
  Uint elm_type;
  Uint number_of_tags=3; // 1 for physical entity,  1 for elementary geometrical entity,  1 for mesh partition
  Uint elm_number=0;
  Uint partition_number = mpi::PE::instance().rank();

  boost_foreach(const CEntities& elements, m_mesh->topology().elements_range())
  {
    group_name = elements.parent().full_path().path();
    group_number = m_groupnumber[group_name];

    m_element_start_idx[&elements]=elm_number;

    //file << "// Region " << elements.full_path().string() << "\n";
    elm_type = m_elementTypes[elements.element_type().builder_name()];
    const Uint nb_elem = elements.size();
    for (Uint e=0; e<nb_elem; ++e, ++elm_number)
    {
      file << elm_number+1 << " " << elm_type << " " << number_of_tags << " " << group_number << " " << 0 << " " << partition_number;
      boost_foreach(const Uint node_idx, elements.get_nodes(e))
      {
        file << " " << node_idx+1;
      }
      file << "\n";
    }
  }
  file << "$EndElements\n";
}

//////////////////////////////////////////////////////////////////////


void CWriter::write_elem_nodal_data(std::fstream& file)
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

  boost_foreach(boost::weak_ptr<CField> field_ptr, m_fields)
  {
    CField& elementbased_field = *field_ptr.lock();
    if (elementbased_field.basis() == CField::Basis::ELEMENT_BASED ||
        elementbased_field.basis() == CField::Basis::CELL_BASED    ||
        elementbased_field.basis() == CField::Basis::FACE_BASED    )
    {
      const Real field_time = elementbased_field.property("time").value<Real>();
      const Uint field_iter = elementbased_field.property("iteration").value<Uint>();
      const std::string field_name = elementbased_field.name();
      std::string field_topology = elementbased_field.topology().full_path().path();
      const std::string field_basis = CField::Basis::Convert::instance().to_str(elementbased_field.basis());
      boost::algorithm::replace_first(field_topology,m_mesh->topology().full_path().path(),"");
      Uint nb_elements = 0;
      boost_foreach(CEntities& elements, find_components_recursively<CEntities>(elementbased_field.topology()))
      {
        if (elementbased_field.exists_for_entities(elements))
        {
          nb_elements += elements.size();
        }
      }

      // data_header
      Uint row_idx=0;
      for (Uint iVar=0; iVar<elementbased_field.nb_vars(); ++iVar)
      {
        CField::VarType var_type = elementbased_field.var_type(iVar);
        std::string var_name = elementbased_field.var_name(iVar);

        Uint datasize(var_type);
        switch (var_type)
        {
          case CField::VECTOR_2D:
            datasize=Uint(CField::VECTOR_3D);
            break;
          case CField::TENSOR_2D:
            datasize=Uint(CField::TENSOR_3D);
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


        boost_foreach(CEntities& elements, find_components_recursively<CEntities>(elementbased_field.topology()))
        {
          if (elementbased_field.exists_for_entities(elements))
          {
            CMultiStateFieldView field_view("field_view");
            field_view.initialize(elementbased_field,elements.as_ptr<CEntities>());
            Uint elm_number = m_element_start_idx[&elements];
            Uint local_nb_elms = elements.size();

            const Uint nb_states = field_view.space().nb_states();
            RealMatrix field_data (field_view.space().nb_states(),var_type);

            const Uint nb_nodes = elements.element_type().nb_nodes();

            /// write element
            for (Uint local_elm_idx = 0; local_elm_idx<local_nb_elms; ++local_elm_idx)
            {
              file << ++elm_number << " " << nb_nodes << " ";

              /// set field data
              CMultiStateFieldView::View data_view = field_view[local_elm_idx];

              for (Uint iState=0; iState<nb_states; ++iState)
              {
                for (Uint j=0; j<var_type; ++j)
                  field_data(iState,j) = data_view[iState][row_idx+j];
              }

              for (Uint iNode=0; iNode<nb_nodes; ++iNode)
              {
                /// get element_node local coordinates
                RealVector local_coords = elements.element_type().shape_function().local_coordinates().row(iNode);

                /// evaluate field shape function in element_node
                RealVector node_data = field_view.space().shape_function().value(local_coords)*field_data;
                cf_assert(node_data.size() == var_type);


                if (var_type==CField::TENSOR_2D)
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
                  if (var_type == CField::VECTOR_2D)
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

void CWriter::write_nodal_data(std::fstream& file)
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

  boost_foreach(boost::weak_ptr<CField> field_ptr, m_fields)
  {
    CField& nodebased_field = *field_ptr.lock();

    if (nodebased_field.basis() == CField::Basis::POINT_BASED)
    {
      const std::string field_name = nodebased_field.name();
      std::string field_topology = nodebased_field.topology().full_path().path();
      boost::algorithm::replace_first(field_topology,m_mesh->topology().full_path().path(),"");
      const Real field_time = nodebased_field.property("time").value<Real>();
      const Uint field_iter = nodebased_field.property("iteration").value<Uint>();
      // data_header
      Uint row_idx=0;
      for (Uint iVar=0; iVar<nodebased_field.nb_vars(); ++iVar)
      {
        CField::VarType var_type = nodebased_field.var_type(iVar);
        std::string var_name = nodebased_field.var_name(iVar);
        Uint datasize(var_type);
        switch (var_type)
        {
          case CField::VECTOR_2D:
            datasize=Uint(CField::VECTOR_3D);
            break;
          case CField::TENSOR_2D:
            datasize=Uint(CField::TENSOR_3D);
            break;
          default:
            break;
        }

        RealVector data(datasize);
        Uint nb_nodes = nodebased_field.size();

        file << "$NodeData\n";
        file << 3 << "\n";
        file << "\"" << (var_name == "var" ? field_name+to_str(iVar) : var_name) << "\"\n";
        file << "\"" << field_name << "\"\n";
        file << "\"" << field_topology << "\"\n";
        file << 1 << "\n" << field_time << "\n";
        file << 3 << "\n" << field_iter << "\n" << datasize << "\n" << nb_nodes <<"\n";


        Uint local_node_idx=0;
        const CTable<Real>& field_data = nodebased_field.data();
        //const CList<Uint>& used_nodes = nodebased_field.used_nodes();

        boost_foreach(CTable<Real>::ConstRow field_per_node, field_data.array())
        {
          file << ++local_node_idx << " ";

          if (var_type==CField::TENSOR_2D)
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
            if (var_type == CField::VECTOR_2D)
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

void CWriter::write_element_data(std::fstream& file)
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

  boost_foreach(boost::weak_ptr<CField> field_ptr, m_fields)
  {
    CField& elementbased_field = *field_ptr.lock();
    if (elementbased_field.basis() == CField::Basis::ELEMENT_BASED ||
        elementbased_field.basis() == CField::Basis::CELL_BASED    ||
        elementbased_field.basis() == CField::Basis::FACE_BASED    )
    {
      const Real field_time = elementbased_field.property("time").value<Real>();
      const Uint field_iter = elementbased_field.property("iteration").value<Uint>();
      const std::string field_name = elementbased_field.name();
      std::string field_topology = elementbased_field.topology().full_path().path();
      const std::string field_basis = CField::Basis::Convert::instance().to_str(elementbased_field.basis());
      boost::algorithm::replace_first(field_topology,m_mesh->topology().full_path().path(),"");
      Uint nb_elements = 0;
      boost_foreach(CEntities& field_elements, find_components_recursively<CEntities>(elementbased_field.topology()))
      {
        if (elementbased_field.exists_for_entities(field_elements))
        {
          nb_elements += field_elements.size();
        }
      }

      // data_header
      Uint row_idx=0;
      for (Uint iVar=0; iVar<elementbased_field.nb_vars(); ++iVar)
      {
        CField::VarType var_type = elementbased_field.var_type(iVar);
        std::string var_name = elementbased_field.var_name(iVar);

        Uint datasize(var_type);
        switch (var_type)
        {
          case CField::VECTOR_2D:
            datasize=Uint(CField::VECTOR_3D);
            break;
          case CField::TENSOR_2D:
            datasize=Uint(CField::TENSOR_3D);
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
        boost_foreach(CEntities& field_elements, find_components_recursively<CEntities>(elementbased_field.topology()))
        {
          if (elementbased_field.exists_for_entities(field_elements))
          {
            CFieldView field_view("field_view");
            field_view.initialize(elementbased_field,field_elements.as_ptr<CEntities>());
            Uint elm_number = m_element_start_idx[&field_elements];
            Uint local_nb_elms = field_elements.size();
            for (Uint local_elm_idx = 0; local_elm_idx<local_nb_elms; ++local_elm_idx)
            {
              file << ++elm_number << " " ;
              if (var_type==CField::TENSOR_2D)
              {
                data[0]=field_view[local_elm_idx][row_idx+0];
                data[1]=field_view[local_elm_idx][row_idx+1];
                data[3]=field_view[local_elm_idx][row_idx+2];
                data[4]=field_view[local_elm_idx][row_idx+3];
                for (Uint idx=0; idx<datasize; ++idx)
                  file << " " << data[idx];
              }
              else
              {
                for (Uint idx=row_idx; idx<row_idx+Uint(var_type); ++idx)
                  file << " " << field_view[local_elm_idx][idx];
                if (var_type == CField::VECTOR_2D)
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

//////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF
