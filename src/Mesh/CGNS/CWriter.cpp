// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BoostFilesystem.hpp"

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CGNS/CWriter.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Mesh {
namespace CGNS {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CGNS::CWriter, CMeshWriter, LibCGNS > aCGNSWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const std::string& name )
: CMeshWriter(name),
  Shared()
{

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

  CFdebug << "Opening file " << path.string() << CFendl;
  CALL_CGNS(cg_open(path.string().c_str(),CG_MODE_WRITE,&m_file.idx));

  write_base();

  CFdebug << "Closing file " << path.string() << CFendl;
  CALL_CGNS(cg_close(m_file.idx));

}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_base()
{
  CRegion& base_region = m_mesh->topology();
  m_base.name = base_region.name();
  m_base.cell_dim = m_max_dimensionality;
  m_base.phys_dim = m_coord_dim;
  CFdebug << "Writing base " << m_base.name << CFendl;
  CALL_CGNS(cg_base_write(m_file.idx,m_base.name.c_str(),m_base.cell_dim,m_base.phys_dim,&m_base.idx));

  BOOST_FOREACH(CRegion& zone_region, find_components<CRegion>(base_region))
  {
    write_zone(zone_region);
  }

}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_zone(const CRegion& region)
{
  m_zone.name = region.name();

  m_zone.coord_dim = m_coord_dim;

  m_zone.total_nbVertices = 0;
  BOOST_FOREACH(const CTable<Real>& coordinates, find_components_recursively_with_tag<CTable<Real> >(*region.parent(),"coordinates"))
    m_zone.total_nbVertices += coordinates.size();

  m_zone.nbElements = region.recursive_elements_count();

  m_zone.nbBdryVertices = 0;

  int size[3][1];
  size[0][0] = m_zone.total_nbVertices;
  size[1][0] = m_zone.nbElements;
  size[2][0] = m_zone.nbBdryVertices;

  CFdebug << "Writing zone " << m_zone.name << CFendl;
  CALL_CGNS(cg_zone_write(m_file.idx,m_base.idx,m_zone.name.c_str(),size[0],Unstructured,&m_zone.idx));

  Real* xCoord(NULL);
  Real* yCoord(NULL);
  Real* zCoord(NULL);

  switch (m_zone.coord_dim)
  {
    case 3:
      zCoord = new Real[m_zone.total_nbVertices];
    case 2:
      yCoord = new Real[m_zone.total_nbVertices];
    case 1:
      xCoord = new Real[m_zone.total_nbVertices];
  }

  Uint idx=0;
  BOOST_FOREACH(const CTable<Real>& coordinates, find_components_recursively_with_tag<CTable<Real> >(region,"coordinates"))
  {
    m_global_start_idx[&coordinates] = idx;

    switch (m_zone.coord_dim)
    {
      case 3:
      {
        BOOST_FOREACH(CTable<Real>::ConstRow node, coordinates.array())
        {
          xCoord[idx] = node[XX];
          yCoord[idx] = node[YY];
          zCoord[idx] = node[ZZ];
          idx++;
        }
        break;
      }
      case 2:
      {
        BOOST_FOREACH(CTable<Real>::ConstRow node, coordinates.array())
        {
          xCoord[idx] = node[XX];
          yCoord[idx] = node[YY];
          idx++;
        }
        break;
      }
      case 1:
      {
        BOOST_FOREACH(CTable<Real>::ConstRow node, coordinates.array())
          xCoord[idx++] = node[XX];
        break;
      }
    }
  }


  int cgns_coord_idx;

  switch (m_zone.coord_dim)
  {
    case 3:
    {
      CFdebug << "Writing CoordinateZ" << CFendl;
      CALL_CGNS(cg_coord_write(m_file.idx,m_base.idx,m_zone.idx,RealDouble,"CoordinateZ",zCoord,&cgns_coord_idx));
      delete_ptr(zCoord);
    }
    case 2:
    {
      CFdebug << "Writing CoordinateY" << CFendl;
      CALL_CGNS(cg_coord_write(m_file.idx,m_base.idx,m_zone.idx,RealDouble,"CoordinateY",yCoord,&cgns_coord_idx));
      delete_ptr(yCoord);
    }
    case 1:
    {
      CFdebug << "Writing CoordinateX" << CFendl;
      CALL_CGNS(cg_coord_write(m_file.idx,m_base.idx,m_zone.idx,RealDouble,"CoordinateX",xCoord,&cgns_coord_idx));
      delete_ptr(xCoord);
    }
  }

  GroupsMapType grouped_elements_map;
  BOOST_FOREACH(const CElements& elements, find_components_recursively<CElements>(region))
  {
    grouped_elements_map[elements.parent()->full_path().path()].push_back(elements.as_ptr<CElements const>());
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
  CFactory& sf_factory = *Core::instance().factories()->get_factory<ElementType>();
  std::map<std::string,std::string> builder_name;
	boost_foreach(CBuilder& sf_builder, find_components_recursively<CBuilder>( sf_factory ) )
	{
		ElementType::Ptr sf = sf_builder.build("sf")->as_ptr<ElementType>();
    builder_name[sf->element_type_name()] = sf_builder.name();
	}
  


  CRegion::ConstPtr section_region = grouped_elements[0]->parent()->as_ptr<CRegion const>();

  m_section.name = section_region->name();
  m_section.type = grouped_elements.size() != 1 ? MIXED : m_elemtype_CF_to_CGNS[builder_name[grouped_elements[0]->element_type().element_type_name()]];

  switch (m_section.type)
  {
    case MIXED:
    {
      CGNS_Section mixed_section;
      int total_nbElems = section_region->recursive_elements_count();
      mixed_section.elemStartIdx = m_section.elemEndIdx + 1;
      mixed_section.elemEndIdx = m_section.elemEndIdx + total_nbElems;
      mixed_section.nbBdry = 0; // unsorted boundary

      // If this region is a surface, it must be a boundary condition.
      // Thus create the boundary condition as an element range (no extra storage)
      if (IsElementsSurface()(*grouped_elements[0]))
      {
        m_boco.name = m_section.name;
        m_section.name = m_section.name + "_bc";
        int range[2];
        range[0] = m_section.elemStartIdx;
        range[1] = m_section.elemEndIdx;
        CFdebug << "Writing boco " << m_boco.name << CFendl;
        cg_boco_write(m_file.idx,m_base.idx,m_zone.idx,m_boco.name.c_str(),BCTypeNull,ElementRange,2,range,&m_boco.idx);
      }


      CALL_CGNS(cg_section_partial_write(m_file.idx,m_base.idx,m_zone.idx,m_section.name.c_str(),m_section.type,mixed_section.elemStartIdx,mixed_section.elemEndIdx,mixed_section.nbBdry,&m_section.idx));

      BOOST_FOREACH(CElements::ConstPtr elements, grouped_elements)
      {
        int nbElems = elements->size();
        m_section.elemNodeCount = elements->element_type().nb_nodes();
        m_section.elemStartIdx = m_section.elemEndIdx + 1;
        m_section.elemEndIdx = m_section.elemEndIdx + nbElems;
        m_section.nbBdry = 0; // unsorted boundary

        ElementType_t type = m_elemtype_CF_to_CGNS[builder_name[elements->element_type().element_type_name()]];
        const CConnectivity::ArrayT& connectivity_table = elements->node_connectivity().array();
        int start_idx = m_global_start_idx[&elements->nodes().coordinates()];

        int* elemNodes = new int [nbElems*(m_section.elemNodeCount+1)];
        for (int iElem=0; iElem<nbElems; ++iElem)
        {
          elemNodes[0 + iElem*(m_section.elemNodeCount+1)] = type;
          for (int iNode=0; iNode<m_section.elemNodeCount; ++iNode)
          {
            elemNodes[1+iNode+ iElem*(m_section.elemNodeCount+1)] = start_idx+connectivity_table[iElem][iNode]+1;
          }
        }

        CALL_CGNS(cg_elements_partial_write(m_file.idx,m_base.idx,m_zone.idx,m_section.idx,m_section.elemStartIdx,m_section.elemEndIdx,elemNodes));

        delete_ptr(elemNodes);
      }
      break;
    }
    default:
    {
      const CElements& elements = *grouped_elements[0];
      int nbElems = elements.size();
      m_section.elemNodeCount = elements.element_type().nb_nodes();
      m_section.elemStartIdx = m_section.elemEndIdx + 1;
      m_section.elemEndIdx = m_section.elemEndIdx + nbElems;
      m_section.nbBdry = 0; // unsorted boundary

      const CConnectivity::ArrayT& connectivity_table = elements.node_connectivity().array();
      int start_idx = m_global_start_idx[&elements.nodes().coordinates()];

      int* elemNodes = new int [nbElems*m_section.elemNodeCount];
      for (int iElem=0; iElem<nbElems; ++iElem)
      {
        for (int iNode=0; iNode<m_section.elemNodeCount; ++iNode)
        {
          elemNodes[iNode+iElem*m_section.elemNodeCount] = start_idx + connectivity_table[iElem][iNode]+1;
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
        CFdebug << "Writing boco " << m_boco.name << CFendl;
        cg_boco_write(m_file.idx,m_base.idx,m_zone.idx,m_boco.name.c_str(),BCTypeNull,ElementRange,2,range,&m_boco.idx);
      }

      CFdebug << "Writing section " << m_section.name << " of type " << m_elemtype_CGNS_to_CF[m_section.type] << CFendl;
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
