#include <boost/foreach.hpp>
#include <boost/filesystem/fstream.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/Gmsh/CWriter.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"

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
  m_elementTypes[GeoShape::HEXA]=5;
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
  CArray::Ptr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");
  const Uint dimension(coordinates->get_array().shape()[1]);
  Uint phys_name_counter(0);
  CFinfo << "searching physical regions" << CFendl;
  BOOST_FOREACH(const CRegion& groupRegion, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
  {
    ++phys_name_counter;
    PhysicalGroup group (dimension,phys_name_counter,groupRegion.name());
    m_groups.insert(PhysicalGroupMap::value_type(group.name,group));
    CFinfo << "   " << groupRegion.name() << CFendl;
  }
  CFinfo << "found: " << phys_name_counter <<CFendl;
  
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
  CArray::Ptr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");

  const Uint coord_dim = coordinates->get_array().shape()[1];
  file << "$Nodes\n";
  file << coordinates->get_array().size() << "\n";
  
  Uint node_number = 0;
  BOOST_FOREACH(CArray::Row row, coordinates->get_array())
  {
    node_number++;
    file << node_number << " ";
    for (Uint d=0; d<3; d++)
    {
      if (d<coord_dim)
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
  CRegion::Ptr regions = get_named_component_typed_ptr<CRegion>(*m_mesh, "regions");
  Uint nbElems = 0;

  BOOST_FOREACH(const CRegion& region, recursive_range_typed<CRegion>(*regions))
  {
    if (range_typed<CRegion>(region).empty())
    {
      nbElems += get_named_component_typed<CTable>(region, "table").get_table().size();
    }
  }
  file << "$Elements\n";
  file << nbElems << "\n";
  std::string group_name("");
  Uint group_number;
  Uint elm_number=0;
  Uint elm_type;
  Uint number_of_tags=2;

  BOOST_FOREACH(CRegion& region, recursive_range_typed<CRegion>(*regions))
  {
    if (!range_typed<CRegion>(region).empty())
    {
      group_name = region.name();
      group_number = m_groups[group_name].number;
    }
    else
    {
      cf_assert(group_name != "");
      elm_type = m_elementTypes[get_named_component_typed<CElements>(region, "type").get_elementType()->getShape()];
      BOOST_FOREACH(CTable::Row row, get_named_component_typed<CTable>(region, "table").get_table())
      {
        elm_number++;
        file << elm_number << " " << elm_type << " " << number_of_tags << " " << group_number << " " << group_number;
        BOOST_FOREACH(Uint node, row)
          file << " " << node+1;
        file << "\n";
      }
    }
  }
  file << "$EndElements\n";
}

//////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF
