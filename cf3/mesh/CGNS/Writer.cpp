// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BoostFilesystem.hpp"

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Table.hpp"

#include "mesh/CGNS/Writer.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace mesh {
namespace CGNS {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CGNS::Writer, MeshWriter, LibCGNS > aCGNSWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

Writer::Writer( const std::string& name )
: MeshWriter(name),
  Shared()
{

}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Writer::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".cgns");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void Writer::write()
{
  m_fileBasename = m_file_path.base_name(); // filename without extension


  CFdebug << "Opening file " << m_file_path.path() << CFendl;
  CALL_CGNS(cg_open(m_file_path.path().c_str(),CG_MODE_WRITE,&m_file.idx));

  write_base(*m_mesh);

  CFdebug << "Closing file " << m_file_path.path() << CFendl;
  CALL_CGNS(cg_close(m_file.idx));

}

/////////////////////////////////////////////////////////////////////////////

void Writer::write_base(const Mesh& mesh)
{
  const Region& base_region = mesh.topology();
  m_base.name = mesh.name();
  m_base.cell_dim = mesh.dimensionality();
  m_base.phys_dim = mesh.dimension();
  CFdebug << "Writing base " << m_base.name << CFendl;
  CALL_CGNS(cg_base_write(m_file.idx,m_base.name.c_str(),m_base.cell_dim,m_base.phys_dim,&m_base.idx));

  //BOOST_FOREACH(const Region& zone_region, find_components<Region>(base_region))
  //{
  write_zone(mesh.topology(), mesh);
  //}

}

/////////////////////////////////////////////////////////////////////////////

void Writer::write_zone(const Region& region, const Mesh& mesh)
{
  m_zone.name = region.name();

  m_zone.coord_dim = mesh.dimension();

  m_zone.total_nbVertices = 0;
  BOOST_FOREACH(const common::Table<Real>& coordinates, find_components_recursively_with_tag<common::Table<Real> >(mesh.geometry_fields(),mesh::Tags::coordinates()))
    m_zone.total_nbVertices += coordinates.size();

  m_zone.nbElements = region.recursive_filtered_elements_count(IsElementsVolume(),true);

  m_zone.nbBdryVertices = 0;

  cgsize_t size[3][1];
  size[0][0] = m_zone.total_nbVertices;
  size[1][0] = m_zone.nbElements;
  size[2][0] = m_zone.nbBdryVertices;

  CFdebug << "Writing zone " << m_zone.name << CFendl;
  CALL_CGNS(cg_zone_write(m_file.idx,m_base.idx,m_zone.name.c_str(),size[0],CGNS_ENUMV( Unstructured ),&m_zone.idx));

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
  BOOST_FOREACH(const common::Table<Real>& coordinates, find_components_recursively_with_tag<common::Table<Real> >(mesh.geometry_fields(),mesh::Tags::coordinates()))
  {
    m_global_start_idx[&coordinates] = idx;

    switch (m_zone.coord_dim)
    {
      case 3:
      {
        BOOST_FOREACH(common::Table<Real>::ConstRow node, coordinates.array())
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
        BOOST_FOREACH(common::Table<Real>::ConstRow node, coordinates.array())
        {
          xCoord[idx] = node[XX];
          yCoord[idx] = node[YY];
          idx++;
        }
        break;
      }
      case 1:
      {
        BOOST_FOREACH(common::Table<Real>::ConstRow node, coordinates.array())
          xCoord[idx++] = node[XX];
        break;
      }
    }
  }

  int cgns_coord_idx;

  if (m_zone.coord_dim > 0)
  {
    CFdebug << "Writing CoordinateX" << CFendl;
    CALL_CGNS(cg_coord_write(m_file.idx,m_base.idx,m_zone.idx,CGNS_ENUMV( RealDouble ),"CoordinateX",xCoord,&cgns_coord_idx));
    delete_ptr(xCoord);
  }
  if (m_zone.coord_dim > 1)
  {
    CFdebug << "Writing CoordinateY" << CFendl;
    CALL_CGNS(cg_coord_write(m_file.idx,m_base.idx,m_zone.idx,CGNS_ENUMV( RealDouble ),"CoordinateY",yCoord,&cgns_coord_idx));
    delete_ptr(yCoord);
  }
  if (m_zone.coord_dim > 2)
  {
    CFdebug << "Writing CoordinateZ" << CFendl;
    CALL_CGNS(cg_coord_write(m_file.idx,m_base.idx,m_zone.idx,CGNS_ENUMV( RealDouble ),"CoordinateZ",zCoord,&cgns_coord_idx));
    delete_ptr(zCoord);
  }

  GroupsMapType grouped_elements_map;
  BOOST_FOREACH(const Elements& elements, find_components_recursively<Elements>(region))
  {
    grouped_elements_map[elements.parent()->uri().path()].push_back(elements.handle<Elements>());
  }

  m_section.elemStartIdx = 0;
  m_section.elemEndIdx = 0;
  BOOST_FOREACH(const GroupsMapType::value_type& grouped_elements, grouped_elements_map)
  {
    write_section(grouped_elements.second);
  }

}

/////////////////////////////////////////////////////////////////////////////

void Writer::write_section(const GroupedElements& grouped_elements)
{
  Factory& sf_factory = *Core::instance().factories().get_factory<ElementType>();
  std::map<std::string,std::string> builder_name;
  boost_foreach(Builder& sf_builder, find_components_recursively<Builder>( sf_factory ) )
  {
    boost::shared_ptr< ElementType > sf = boost::dynamic_pointer_cast<ElementType>(sf_builder.build("sf"));
    builder_name[sf->derived_type_name()] = sf_builder.name();
  }



  Handle<Region const> section_region = Handle<Region const>(grouped_elements[0]->parent());

  m_section.name = section_region->name();
  m_section.type = grouped_elements.size() != 1 ? CGNS_ENUMV( MIXED ) : m_elemtype_CF3_to_CGNS[builder_name[grouped_elements[0]->element_type().derived_type_name()]];

  switch (m_section.type)
  {
    case CGNS_ENUMV( MIXED ):
    {
      CGNS_Section mixed_section;
      int total_nbElems = section_region->recursive_elements_count(true);
      mixed_section.elemStartIdx = m_section.elemEndIdx + 1;
      mixed_section.elemEndIdx = m_section.elemEndIdx + total_nbElems;
      mixed_section.nbBdry = 0; // unsorted boundary

      // If this region is a surface, it must be a boundary condition.
      // Thus create the boundary condition as an element range (no extra storage)
      if (IsElementsSurface()(*grouped_elements[0]))
      {
        m_boco.name = m_section.name;
        m_section.name = m_section.name + "_bc";
        cgsize_t range[2];
        range[0] = m_section.elemStartIdx;
        range[1] = m_section.elemEndIdx;
        CFdebug << "Writing boco " << m_boco.name << CFendl;
        cg_boco_write(m_file.idx,m_base.idx,m_zone.idx,m_boco.name.c_str(),CGNS_ENUMV( BCTypeNull ),CGNS_ENUMV( ElementRange ),2,range,&m_boco.idx);
      }


      CALL_CGNS(cg_section_partial_write(m_file.idx,m_base.idx,m_zone.idx,m_section.name.c_str(),m_section.type,mixed_section.elemStartIdx,mixed_section.elemEndIdx,mixed_section.nbBdry,&m_section.idx));

      BOOST_FOREACH(const Handle< Elements const>& elements, grouped_elements)
      {
        int nbElems = elements->size();
        m_section.elemNodeCount = elements->element_type().nb_nodes();
        m_section.elemStartIdx = m_section.elemEndIdx + 1;
        m_section.elemEndIdx = m_section.elemEndIdx + nbElems;
        m_section.nbBdry = 0; // unsorted boundary

        CGNS_ENUMT( ElementType_t ) type = m_elemtype_CF3_to_CGNS[builder_name[elements->element_type().derived_type_name()]];
        const Connectivity::ArrayT& connectivity_table = elements->geometry_space().connectivity().array();
        int start_idx = m_global_start_idx[&elements->geometry_fields().coordinates()];

        cgsize_t* elemNodes = new cgsize_t [nbElems*(m_section.elemNodeCount+1)];
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
      const Elements& elements = *grouped_elements[0];
      int nbElems = elements.size();
      m_section.elemNodeCount = elements.element_type().nb_nodes();
      m_section.elemStartIdx = m_section.elemEndIdx + 1;
      m_section.elemEndIdx = m_section.elemEndIdx + nbElems;
      m_section.nbBdry = 0; // unsorted boundary

      const Connectivity::ArrayT& connectivity_table = elements.geometry_space().connectivity().array();
      int start_idx = m_global_start_idx[&elements.geometry_fields().coordinates()];

      cgsize_t* elemNodes = new cgsize_t [nbElems*m_section.elemNodeCount];
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
        cgsize_t range[2];
        range[0] = m_section.elemStartIdx;
        range[1] = m_section.elemEndIdx;
        CFdebug << "Writing boco " << m_boco.name << CFendl;
        cg_boco_write(m_file.idx,m_base.idx,m_zone.idx,m_boco.name.c_str(),CGNS_ENUMV( BCTypeNull ),CGNS_ENUMV( ElementRange ),2,range,&m_boco.idx);
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
} // mesh
} // cf3
