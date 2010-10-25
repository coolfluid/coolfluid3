// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/String/Conversion.hpp"

#include "Mesh/Gmsh/CWriter.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
	
	using namespace Common::String;
	
namespace Mesh {
namespace Gmsh {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Mesh::Gmsh::CWriter,
                         Mesh::CMeshWriter,
                         Mesh::Gmsh::LibGmsh,
                         1 >
aGmshWriter_Provider ( "Gmsh" );

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const CName& name )
: CMeshWriter(name)
{
  BUILD_COMPONENT;

  // gmsh types: http://www.geuz.org/gmsh/doc/texinfo/gmsh.html#MSH-ASCII-file-format
  m_elementTypes["Line1DLagrangeP1" ]=1;
	m_elementTypes["Line2DLagrangeP1" ]=1;
	m_elementTypes["Line3DLagrangeP1" ]=1;
	m_elementTypes["Triag2DLagrangeP1"]=2;
	m_elementTypes["Triag3DLagrangeP1"]=2;
	m_elementTypes["Quad2DLagrangeP1" ]=3;
	m_elementTypes["Quad3DLagrangeP1" ]=3;
	m_elementTypes["Tetra3DLagrangeP1"]=4;
	m_elementTypes["Hexa3DLagrangeP1" ]=5;

	m_elementTypes["Triag2DLagrangeP2"]=9;
	m_elementTypes["Triag3DLagrangeP2"]=9;
	m_elementTypes["Quad2DLagrangeP2" ]=10;
	m_elementTypes["Quad3DLagrangeP2" ]=10;

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

void CWriter::write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path)
{

  m_mesh = mesh;

  // if the file is present open it
  boost::filesystem::fstream file;
	if (PEInterface::instance().size() > 1)
	{
		path = boost::filesystem::basename(path) + "_P" + to_str(PEInterface::instance().rank()) + boost::filesystem::extension(path);
	}
  CFLog(VERBOSE, "Opening file " <<  path.string() << "\n");
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
	//write_nodal_data(file);
  write_element_data(file);
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


  // physical names
  Uint phys_name_counter(0);
  BOOST_FOREACH(const CRegion& groupRegion, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
  {
    ++phys_name_counter;
    PhysicalGroup group (m_coord_dim,phys_name_counter,groupRegion.full_path().string());
    m_groups.insert(PhysicalGroupMap::value_type(group.name,group));
  }

  file << "$PhysicalNames\n";
  file << phys_name_counter << "\n";
  BOOST_FOREACH(PhysicalGroupMap::value_type& g, m_groups)
    file << g.second.dimension << " " << g.second.number << " \"" << g.second.name << "\"\n";
  file << "$EndPhysicalNames\n";
}

//////////////////////////////////////////////////////////////////////////////

void CWriter::write_coordinates(std::fstream& file)
{
  // set precision for Real
  Uint prec = file.precision();
  file.precision(8);

  Uint nb_nodes(0);
  BOOST_FOREACH(CoordinatesElementsMap::value_type& coord, m_all_coordinates)
  {
    nb_nodes += coord.first->size();
  }


  file << "$Nodes\n";
  file << nb_nodes << "\n";

  Uint node_number = 0;

  BOOST_FOREACH(CoordinatesElementsMap::value_type& coord, m_all_coordinates)
  {
    BOOST_FOREACH(CElements* elements, coord.second)
      m_node_start_idx[elements] = node_number;

    BOOST_FOREACH(CArray::ConstRow row, coord.first->array())
    {
      ++node_number;
      file << node_number << " ";
      for (Uint d=0; d<3; d++)
      {
        if (d<m_coord_dim)
          file << row[d] << " ";
        else
          file << 0 << " ";
      }
      file << "\n";
    }
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
  Uint nbElems = 0;
  BOOST_FOREACH(const CRegion& region, range_typed<CRegion>(*m_mesh))
  {
    nbElems += region.recursive_elements_count();
  }

  file << "$Elements\n";
  file << nbElems << "\n";
  std::string group_name("");
  Uint group_number;
  Uint elm_type;
  Uint number_of_tags=2;
  Uint elm_number=0;

//  BOOST_FOREACH(const CRegion& region, recursive_range_typed<CRegion>(*m_mesh))
//  {
//      group_name = region.name();
//      group_number = m_groups[group_name].number;
//  }

  BOOST_FOREACH(const CoordinatesElementsMap::value_type& coord, m_all_coordinates)
  {
    BOOST_FOREACH(CElements* elements, coord.second)
    {
      if (!elements->has_tag("CFieldElements"))
      {
        group_name = elements->get_parent()->full_path().string();
        group_number = m_groups[group_name].number;

        m_element_start_idx[elements]=elm_number;

        //file << "// Region " << elements.full_path().string() << "\n";
        elm_type = m_elementTypes[elements->element_type().getElementTypeName()];
        Uint node_start_idx = m_node_start_idx[elements];
        BOOST_FOREACH(const CTable::ConstRow& row, elements->connectivity_table().array())
        {
          elm_number++;
          file << elm_number << " " << elm_type << " " << number_of_tags << " " << group_number << " " << group_number;
          BOOST_FOREACH(const Uint local_node_idx, row)
          {
            file << " " << node_start_idx+local_node_idx+1;
          }
          file << "\n";
        }
      }
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

  BOOST_FOREACH(CField& nodebased_field, filtered_range_typed<CField>(*m_mesh,IsFieldNodeBased()))
  {
    std::string field_name = nodebased_field.field_name();
    Uint nb_elements = nodebased_field.support().recursive_elements_count();

		
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
      data.setZero();

			
			file << "$ElementNodeData\n";
			file << 1 << "\n";
			file << "\"" << (var_name == "var" ? field_name+to_str(iVar) : var_name) << "\"\n";
			file << 1 << "\n" << 0.0 << "\n";
			file << 3 << "\n" << 0 << "\n" << datasize << "\n" << nb_elements <<"\n";
			
			BOOST_FOREACH(CFieldElements& field_elements, recursive_range_typed<CFieldElements>(nodebased_field))
			{
				const CArray& field_data = field_elements.data();
				Uint nb_nodes_per_element = field_elements.element_type().nb_nodes();
				
				Uint elm_number = m_element_start_idx[&field_elements.get_geometry_elements()];
				BOOST_FOREACH(const CTable::ConstRow& row, field_elements.connectivity_table().array())
				{
					elm_number++;
					file << elm_number << " " << nb_nodes_per_element << " ";
					BOOST_FOREACH(const Uint local_node_idx, row)
					{
						if (var_type==CField::TENSOR_2D)
						{
							data[0]=field_data[local_node_idx][row_idx+0];
							data[1]=field_data[local_node_idx][row_idx+1];
							data[3]=field_data[local_node_idx][row_idx+2];
							data[4]=field_data[local_node_idx][row_idx+3];
							for (Uint idx=0; idx<datasize; ++idx)
								file << " " << data[idx];
						}
						else
						{
							for (Uint idx=row_idx; idx<row_idx+Uint(var_type); ++idx)
								file << " " << field_data[local_node_idx][idx];
							if (var_type == CField::VECTOR_2D)
								file << " " << 0.0;
						}
					}
					file << "\n";
				}
			}
			file << "$EndElementNodeData\n";
			row_idx += Uint(var_type);
		}			
  }
  // restore precision
  file.precision(prec);
}

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
	
	BOOST_FOREACH(CField& nodebased_field, filtered_range_typed<CField>(*m_mesh,IsFieldNodeBased()))
	{
		std::string field_name = nodebased_field.field_name();		
		
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
			RealVector data(0.0, datasize); 
			
			std::set<std::string> field_data_paths;
			BOOST_FOREACH(CFieldElements& field_elements, recursive_range_typed<CFieldElements>(nodebased_field))
			{
				field_data_paths.insert(field_elements.data().full_path().string());
			}
			
			Uint nb_nodes=0;
			BOOST_FOREACH(const CPath& field_data_path, field_data_paths)
			{
				const CArray& field_data = *m_mesh->look_component_type<CArray>(field_data_path);
				
				nb_nodes += field_data.size();
			}
			
			file << "$NodeData\n";
			file << 1 << "\n";
			file << "\"" << (var_name == "var" ? field_name+to_str(iVar) : var_name) << "\"\n";
			file << 1 << "\n" << 0.0 << "\n";
			file << 3 << "\n" << 0 << "\n" << datasize << "\n" << nb_nodes <<"\n";
			
			
			
			Uint local_node_idx=1;   // +1 for gmsh base starting at 1
			BOOST_FOREACH(const CPath& field_data_path, field_data_paths)
			{
				const CArray& field_data = *m_mesh->look_component_type<CArray>(field_data_path);

				BOOST_FOREACH(CArray::ConstRow field_per_node, field_data.array())
				{
					file << local_node_idx++ << " "; 

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
			}
			file << "$EndNodeData\n";
			row_idx += Uint(var_type);
		}			
	}
	// restore precision
	file.precision(prec);
}
	
//////////////////////////////////////////////////////////////////////

void CWriter::write_element_data(std::fstream& file)
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

  BOOST_FOREACH(CField& elementbased_field, filtered_range_typed<CField>(*m_mesh,IsFieldElementBased()))
  {
    std::string field_name = elementbased_field.field_name();
    Uint nb_elements = elementbased_field.support().recursive_elements_count();

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
			file << 1 << "\n";
			file << "\"" << (var_name == "var" ? field_name+to_str(iVar) : var_name) << "\"\n";
			file << 1 << "\n" << 0.0 << "\n";
			file << 3 << "\n" << 0 << "\n" << datasize << "\n" << nb_elements <<"\n";
			BOOST_FOREACH(CFieldElements& field_elements, recursive_range_typed<CFieldElements>(elementbased_field))
			{
				const CArray& field_data = field_elements.data();
				
				Uint elm_number = m_element_start_idx[&field_elements.get_geometry_elements()];
				Uint local_nb_elms = field_elements.connectivity_table().size();
				for (Uint local_elm_idx = 0; local_elm_idx<local_nb_elms; ++local_elm_idx)
				{
					file << ++elm_number << " " ;
					if (var_type==CField::TENSOR_2D)
					{
						data[0]=field_data[local_elm_idx][row_idx+0];
						data[1]=field_data[local_elm_idx][row_idx+1];
						data[3]=field_data[local_elm_idx][row_idx+2];
						data[4]=field_data[local_elm_idx][row_idx+3];
						for (Uint idx=0; idx<datasize; ++idx)
							file << " " << data[idx];
					}
					else
					{
						for (Uint idx=row_idx; idx<row_idx+Uint(var_type); ++idx)
							file << " " << field_data[local_elm_idx][idx];
						if (var_type == CField::VECTOR_2D)
							file << " " << 0.0;
					}						
					file << "\n";
				}
			}
			file << "$EndElementData\n";
			row_idx += Uint(var_type);
		}			
	}
  // restore precision
  file.precision(prec);
}

//////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF
