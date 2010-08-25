#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CGNS/CWriter.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Mesh::CGNS::CWriter,
                         Mesh::CMeshWriter,
                         Mesh::CGNS::CGNSLib,
                         1 >
aCGNSWriter_Provider ( "CGNS" );

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const CName& name )
: CMeshWriter(name),
  Shared()
{
  BUILD_COMPONENT;
}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CWriter::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".cgns");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path)
{
  m_mesh = mesh;

  m_fileBasename = boost::filesystem::basename(path);

  compute_mesh_specifics();
    
  CFinfo << "Opening file " << path.string() << CFendl;
  CALL_CGNS(cg_open(path.string().c_str(),CG_MODE_WRITE,&m_file.idx));
  
  write_base();
  
  CFinfo << "Closing file " << path.string() << CFendl;
  CALL_CGNS(cg_close(m_file.idx));

}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_base()
{
  CRegion& base_region = m_mesh->geometry();
  m_base.name = base_region.name();
  m_base.cell_dim = m_max_dimensionality;
  m_base.phys_dim = m_coord_dim;
  CFinfo << "Writing base " << m_base.name << CFendl;
  CALL_CGNS(cg_base_write(m_file.idx,m_base.name.c_str(),m_base.cell_dim,m_base.phys_dim,&m_base.idx));
  
  if (base_region.has_tag("grid_zone"))
  {
    write_zone(base_region);
  }
  else if (base_region.has_tag("grid_base"))
  {
    BOOST_FOREACH(CRegion& zone_region, filtered_range_typed<CRegion>(base_region,IsComponentTag("grid_zone")))
    {
      write_zone(zone_region);
    }
  
  }
}
  
/////////////////////////////////////////////////////////////////////////////

void CWriter::write_zone(const CRegion& region)
{
  m_zone.name = region.name();
  
  m_zone.coord_dim = m_coord_dim;
  
  m_zone.total_nbVertices = 0;
  BOOST_FOREACH(const CArray& coordinates, recursive_filtered_range_typed<CArray>(region,IsComponentTag("coordinates")))
    m_zone.total_nbVertices += coordinates.size();

  m_zone.nbElements = region.recursive_elements_count();
  
  m_zone.nbBdryVertices = 0;
  
  int size[3][1];
  size[0][0] = m_zone.total_nbVertices; 
  size[1][0] = m_zone.nbElements; 
  size[2][0] = m_zone.nbBdryVertices; 
  
  CFinfo << "Writing zone " << m_zone.name << CFendl;
  CALL_CGNS(cg_zone_write(m_file.idx,m_base.idx,m_zone.name.c_str(),size[0],Unstructured,&m_zone.idx));

  // check for all coordinates
  if (count(recursive_filtered_range_typed<CArray>(region,IsComponentTag("coordinates"))) == 1)
  {
    const CArray::Array& coordinates = recursive_get_component_typed<CArray>(region,IsComponentTag("coordinates")).array();
    
    int coord_idx;
    switch (m_zone.coord_dim)
    {
      case 3:
      {
        CFinfo << "Writing CoordinateZ" << CFendl;
        Real* zCoord = new Real[m_zone.total_nbVertices];
        Uint i=0;
        BOOST_FOREACH(CArray::ConstRow node, coordinates)
          zCoord[i++] = node[ZZ];
        CALL_CGNS(cg_coord_write(m_file.idx,m_base.idx,m_zone.idx,RealDouble,"CoordinateZ",zCoord,&coord_idx));
        delete_ptr(zCoord);
      }
      case 2:
      {
        CFinfo << "Writing CoordinateY" << CFendl;
        Real* yCoord = new Real[m_zone.total_nbVertices];
        Uint i=0;
        BOOST_FOREACH(CArray::ConstRow node, coordinates)
          yCoord[i++] = node[YY];        
        CALL_CGNS(cg_coord_write(m_file.idx,m_base.idx,m_zone.idx,RealDouble,"CoordinateY",yCoord,&coord_idx));
        delete_ptr(yCoord);
      }
      case 1:
      {
        CFinfo << "Writing CoordinateX" << CFendl;
        Real* xCoord = new Real[m_zone.total_nbVertices];
        Uint i=0;
        BOOST_FOREACH(CArray::ConstRow node, coordinates)
          xCoord[i++] = node[XX];
        CALL_CGNS(cg_coord_write(m_file.idx,m_base.idx,m_zone.idx,RealDouble,"CoordinateX",xCoord,&coord_idx));
        delete_ptr(xCoord);
      }
    }
    
  }
  else
  {
    CFinfo << "There are " << count(recursive_filtered_range_typed<CArray>(region,IsComponentTag("coordinates"))) << "coordinate components in this zone" << CFendl; 
    throw NotImplemented(FromHere(), "no multiple coordinate components supported for writing to cgns");
  }
  
  GroupsMapType grouped_elements_map;
  BOOST_FOREACH(const CElements& elements, recursive_range_typed<CElements>(region))
  {
    grouped_elements_map[elements.get_parent()->full_path().string()].push_back(elements.get_type<CElements const>());
  }
  
  m_section.elemStartIdx = 0;
  m_section.elemEndIdx = 0;
  BOOST_FOREACH(const GroupsMapType::value_type& grouped_elements, grouped_elements_map)
  {
    write_section(grouped_elements.second);
  }

}
  
/////////////////////////////////////////////////////////////////////////////

void CWriter::write_section(const GroupedElements& grouped_elements)
{
  m_section.name = grouped_elements[0]->get_parent()->name();
  m_section.type = grouped_elements.size() != 1 ? MIXED : m_elemtype_CF_to_CGNS[grouped_elements[0]->element_type().getElementTypeName()];
  
  
  switch (m_section.type)
  {
    case MIXED:
    {
      throw NotImplemented (FromHere(), "MIXED element type not supported to write yet");
      break;
    }
    default:
    {
      const CElements& elements = *grouped_elements[0];
      int nbElems = elements.elements_count();
      m_section.elemNodeCount = elements.element_type().nb_nodes();
      m_section.elemStartIdx = m_section.elemEndIdx + 1;
      m_section.elemEndIdx = m_section.elemEndIdx + nbElems;
      m_section.nbBdry = 0; // unsorted boundary
      
      const CTable::ConnectivityTable& connectivity_table = elements.connectivity_table().table();
      
      int* elemNodes = new int [nbElems*m_section.elemNodeCount];
      for (int iElem=0; iElem<nbElems; ++iElem)
      {
        for (int iNode=0; iNode<m_section.elemNodeCount; ++iNode)
        {
          elemNodes[iNode+iElem*m_section.elemNodeCount] = connectivity_table[iElem][iNode]+1;
        }
      }
      
      // If this region is a surface, it must be a boundary condition.
      // Thus create the boundary condition as an element range (no extra storage)
      if (IsElementsSurface()(elements))
      {
        m_boco.name = m_section.name;
        m_section.name = m_section.name + "_bc";
        int range[2];
        range[0] = m_section.elemStartIdx;
        range[1] = m_section.elemEndIdx;
        cg_boco_write(m_file.idx,m_base.idx,m_zone.idx,m_boco.name.c_str(),BCTypeNull,ElementRange,2,range,&m_boco.idx);        
      }
      
      CFinfo << "Writing section " << m_section.name << " of type " << m_elemtype_CGNS_to_CF[m_section.type] << CFendl;
      CALL_CGNS(cg_section_write(m_file.idx,m_base.idx,m_zone.idx,m_section.name.c_str(),m_section.type,m_section.elemStartIdx,  \
                                 m_section.elemEndIdx,m_section.nbBdry,elemNodes,&m_section.idx));
      delete_ptr(elemNodes);
    }
  }
  


}
  
//////////////////////////////////////////////////////////////////////////////


} // CGNS
} // Mesh
} // CF
