#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/Gmsh/Writer.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Writer,
                         Mesh::MeshWriter,
                         GmshLib >
aGmshWriter_Provider ( "Mesh::Gmsh::Writer" );

//////////////////////////////////////////////////////////////////////////////

Writer::Writer()
: MeshWriter()
{
  // gmsh types: http://www.geuz.org/gmsh/doc/texinfo/gmsh.html#MSH-ASCII-file-format
  m_elementTypes[GeoShape::TRIAG]=2;
  m_elementTypes[GeoShape::QUAD]=3;
  m_elementTypes[GeoShape::HEXA]=5;
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
  
  
  // physical names
  CFinfo << "\n\nWriting physical names \n" << CFendl;
  CArray::Ptr coordinates = m_mesh->get_component<CArray>("coordinates");
  const Uint dimension(coordinates->get_array().shape()[1]);
  Uint phys_name_counter(0);

  CRegion::Ptr regions = m_mesh->get_component<CRegion>("regions");
  BOOST_FOREACH(const CRegion::Ptr& region, iterate_recursive_by_type<CRegion>(regions))
  {
    if (!region->has_component_of_type<CRegion>())
    {
      bool exists = false;
      for(PhysicalGroupMap::iterator it=m_groups.begin(); it!=m_groups.end(); ++it)
        if (it->first == region->get_parent()->name())
        { exists = true; break; }

      if (!exists)
      {
        ++phys_name_counter;
        PhysicalGroup group (dimension,phys_name_counter,region->get_parent()->name());
        m_groups.insert(PhysicalGroupMap::value_type(group.name,group));
        CFinfo << phys_name_counter << ": " << group.name << "\n" << CFendl;
      }
    }
  }
  
  file << "$PhysicalNames\n";
  file << phys_name_counter << "\n";
  BOOST_FOREACH(PhysicalGroupMap::value_type& g, m_groups)
    file << g.second.dimension << " " << g.second.number << " \"" << g.second.name << "\"\n"; 
  file << "$EndPhysicalNames\n";
}

//////////////////////////////////////////////////////////////////////////////

void Writer::write_coordinates(std::fstream& file)
{
  // set precision for Real
  Uint prec = file.precision();
  file.precision(8);
  CArray::Ptr coordinates = m_mesh->get_component<CArray>("coordinates");

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

void Writer::write_connectivity(std::fstream& file)
{
  // file << "$Elements                                                               \n";
  // file << "number-of-elements                                                      \n";
  // file << "elm-number elm-type number-of-tags < tag > ... node-number-list ...     \n";
  // file << "$EndElements\n";
  CRegion::Ptr regions = m_mesh->get_component<CRegion>("regions");
  Uint nbElems = 0;

  BOOST_FOREACH(const CRegion::Ptr& region, iterate_recursive_by_type<CRegion>(regions))
  {
    if (!region->has_component_of_type<CRegion>())
    {
      nbElems += region->get_component<CTable>("table")->get_table().size();
    }
  }
  file << "$Elements\n";
  file << nbElems << "\n";
  std::string group_name("");
  Uint group_number;
  Uint elm_number=0;
  Uint elm_type;
  Uint number_of_tags=2;

  BOOST_FOREACH(const CRegion::Ptr& region, iterate_recursive_by_type<CRegion>(regions))
  {
    if (region->has_component_of_type<CRegion>())
    {
      group_name = region->name();
      group_number = m_groups[group_name].number;
    }
    else
    {
      cf_assert(group_name != "");
      elm_type = m_elementTypes[region->get_component<CElements>("type")->get_elementType()->getShape()];
      BOOST_FOREACH(CTable::Row row, region->get_component<CTable>("table")->get_table())
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
