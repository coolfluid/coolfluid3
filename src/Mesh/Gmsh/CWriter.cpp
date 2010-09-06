#include <boost/foreach.hpp>
#include <boost/filesystem/fstream.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/Gmsh/CWriter.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Mesh::Gmsh::CWriter,
                         Mesh::CMeshWriter,
                         Mesh::Gmsh::GmshLib,
                         1 >
aGmshWriter_Provider ( "Gmsh" );

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const CName& name )
: CMeshWriter(name)
{
  BUILD_COMPONENT;

  // gmsh types: http://www.geuz.org/gmsh/doc/texinfo/gmsh.html#MSH-ASCII-file-format
  m_elementTypes[GeoShape::LINE]=1;
  m_elementTypes[GeoShape::TRIAG]=2;
  m_elementTypes[GeoShape::QUAD]=3;
  m_elementTypes[GeoShape::TETRA]=4;
  m_elementTypes[GeoShape::HEXA]=5;
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
  BOOST_FOREACH(const CElements& region, recursive_range_typed<CElements>(*m_mesh))
  {
    nbElems += region.connectivity_table().size();
  }
  file << "$Elements\n";
  file << nbElems << "\n";
  std::string group_name("");
  Uint group_number;
  Uint elm_number=0;
  Uint elm_type;
  Uint number_of_tags=2;

//  BOOST_FOREACH(const CRegion& region, recursive_range_typed<CRegion>(*m_mesh))
//  {
//      group_name = region.name();
//      group_number = m_groups[group_name].number;
//  }
  
  Uint global_node_idx = 0;
  BOOST_FOREACH(const CoordinatesElementsMap::value_type& coord, m_all_coordinates)
  {
    BOOST_FOREACH(const CElements* elements, coord.second)
    {
      group_name = elements->get_parent()->full_path().string();
      group_number = m_groups[group_name].number;

      //file << "// Region " << elements.full_path().string() << "\n";
      elm_type = m_elementTypes[elements->element_type().shape()];
      BOOST_FOREACH(const CTable::ConstRow& row, elements->connectivity_table().array())
      {
        elm_number++;
        file << elm_number << " " << elm_type << " " << number_of_tags << " " << group_number << " " << group_number;
        BOOST_FOREACH(const Uint local_node_idx, row)
        {
          file << " " << global_node_idx+local_node_idx+1;
        }
        file << "\n";
      }
    }
    global_node_idx += coord.first->size();
  }
  file << "$EndElements\n";
}

//////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF
