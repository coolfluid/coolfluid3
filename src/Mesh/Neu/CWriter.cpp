// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>
#include <iomanip>

#include <boost/foreach.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/progress.hpp>

#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/Neu/CWriter.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ConnectivityData.hpp"
#include "Mesh/ElementData.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Mesh {
namespace Neu {
  
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Mesh::Neu::CWriter,
                           Mesh::CMeshWriter,
                           Mesh::Neu::LibNeu>
aNeuWriter_Builder;

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
  extensions.push_back(".neu");
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
  m_fileBasename = boost::filesystem::basename(path);

  compute_mesh_specifics();
  
  // must be in correct order!
  write_headerData(file);
  write_coordinates(file);
  write_connectivity(file);
  write_groups(file);
  write_boundaries(file);

  file.close();

}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_headerData(std::fstream& file)
{
  // get the day of today
  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::gregorian::date::ymd_type today_ymd = today.year_month_day();

  Uint group_counter(0);
  Uint element_counter(0);
  Uint bc_counter(0);

  Uint node_counter(0);
    
  
  
  BOOST_FOREACH(const CoordinatesElementsMap::value_type& coord_array, m_all_coordinates)
    node_counter += coord_array.first->size();
  

  BOOST_FOREACH(const CRegion& group, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
  {
    bool isGroupBC(false);
    BOOST_FOREACH(const CElements& elementregion, recursive_range_typed<CElements>(group))
    {
      bool isElementBC(false);
      Uint dimensionality = elementregion.element_type().dimensionality();
      if (dimensionality < m_max_dimensionality) // is bc
      {
        isElementBC = true;
        isGroupBC = true;
      }
      if (!isElementBC)
        element_counter += elementregion.connectivity_table().size();
    }
    if (!isGroupBC)
    {
      group_counter++;
    }
    else
    {
      bc_counter++;
    }
  }


  file.setf(std::ios::right);
  //CFinfo << "group_counter = " << group_counter << CFendl;
  //CFinfo << "element_counter = " << element_counter << CFendl;
  //CFinfo << "node_counter = " << node_counter << CFendl;
  file << "        CONTROL INFO 2.3.16\n";
  file << "** GAMBIT NEUTRAL FILE\n";
  file << m_fileBasename << "\n";
  file << "PROGRAM:                Gambit     VERSION:  2.3.16\n";
  file << std::setw(4)  << std::string(today_ymd.month.as_long_string()).substr(0,3) << " " << today_ymd.year << "\n";
  file << std::setw(10) << "NUMNP" << std::setw(10) << "NELEM" << std::setw(10) << "NGRPS"
       << std::setw(10) << "NBSETS" << std::setw(10) << "NDFCD" << std::setw(10) << "NDFVL" << std::endl;
  file << std::setw(10) << node_counter << std::setw(10) << element_counter << std::setw(10) << group_counter
       << std::setw(10) << bc_counter << std::setw(10) << m_coord_dim << std::setw(10) << m_coord_dim << std::endl;
  file << "ENDOFSECTION" << std::endl ;
}

//////////////////////////////////////////////////////////////////////////////

void CWriter::write_coordinates(std::fstream& file)
{
  // set precision for Real
  Uint prec = file.precision();
  file.precision(11);
  

  file << "   NODAL COORDINATES 2.3.16" << std::endl;
  file.setf(std::ios::fixed);
  Uint node_number = 0;
  BOOST_FOREACH(const CoordinatesElementsMap::value_type& coord_array, m_all_coordinates)
  {
      BOOST_FOREACH(CTable<Real>::ConstRow row, coord_array.first->array())
      {
        ++node_number;
        file << std::setw(10) << node_number;
        for (Uint d=0; d<m_coord_dim; ++d)
          file << std::setw(20) << std::scientific << row[d];
        file << std::endl;
      }
  }
  file << "ENDOFSECTION" << std::endl;
  // restore precision
  file.precision(prec);
}

//////////////////////////////////////////////////////////////////////////////

void CWriter::write_connectivity(std::fstream& file)
{
  file << "      ELEMENTS/CELLS 2.3.16" << std::endl;
  /// @todo //////////////////////////

  // global element number
  Uint elm_number=0;

  // loop over all element regions
  Uint global_node_idx=0;
  BOOST_FOREACH(const CoordinatesElementsMap::value_type& coord_array, m_all_coordinates)
  {
    BOOST_FOREACH(const CElements* elementregion, coord_array.second)
    {
      bool isBC = false;
      Uint dimensionality = elementregion->element_type().dimensionality();
      if (dimensionality < m_max_dimensionality) // is bc
      {
        isBC = true;
      }
      if (!isBC)
      {
        //CFinfo << "elements from region: " << elementregion->full_path().string() << CFendl;
        // information of this region with one unique element type
        Uint elm_type;
        Uint nb_nodes;
        elm_type = m_CFelement_to_NeuElement[elementregion->element_type().shape()];
        nb_nodes = elementregion->element_type().nb_nodes();
        m_global_start_idx[elementregion]=elm_number;

        // write the nodes for each element of this region
        BOOST_FOREACH(const CTable<Uint>::ConstRow& cf_element , elementregion->connectivity_table().array())
        {
          file << std::setw(8) << ++elm_number << std::setw(3) << elm_type << std::setw(3) << nb_nodes << " ";
          std::vector<Uint> neu_element(nb_nodes);
          
          // fill the neu_element (connectivity)
          for (Uint j=0; j<nb_nodes; ++j)
          {
            // index within a neu element (because of different node numbering)
            Uint neu_idx = m_nodes_cf_to_neu[elm_type][j];
            // put the global element number inside the row
            neu_element[neu_idx] = global_node_idx+cf_element[j]+1;
          }
          
          Uint eol_counter=0;
          BOOST_FOREACH(Uint neu_node, neu_element)
          {
            if (eol_counter == 7)
            {
              file << std::endl << std::setw(15) << " ";
              eol_counter = 0;
            }
            file << std::setw(8) << neu_node;
            ++eol_counter;
          }
          file << std::endl;
        }
      }
    }
    global_node_idx += coord_array.first->size();
  }
  file << "ENDOFSECTION" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

void CWriter::write_groups(std::fstream& file)
{
  Uint group_counter(0);
  
  BOOST_FOREACH(const CRegion& group, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
  {
    bool isBC(false);
    BOOST_FOREACH(const CElements& elementregion, recursive_range_typed <CElements>(group))
    {
      Uint dimensionality = elementregion.element_type().dimensionality();
      if (dimensionality < m_max_dimensionality) // is bc
      {
        isBC = true;
      }
    }
    if (!isBC)
    {
      Uint element_counter(0);
      BOOST_FOREACH(const CElements& elementregion, recursive_range_typed<CElements>(group))
      {
        element_counter += elementregion.connectivity_table().size();
      }
      file << "       ELEMENT GROUP 2.3.16\n";
      file << "GROUP:" << std::setw(11) << ++group_counter << " ELEMENTS:" << std::setw(11) << element_counter << " MATERIAL:" << std::setw(11) << 2 << " NFLAGS:" << std::setw(11) << 1 << std::endl;
      file << std::setw(32) << group.name() << std::endl << std::setw(8) << 0 << std::endl;
      Uint line_counter=0;
      BOOST_FOREACH(const CElements& elementregion, recursive_range_typed <CElements>(group))
      {
        Uint elm_global_start_idx = m_global_start_idx[&elementregion]+1;
        Uint elm_global_end_idx = elementregion.connectivity_table().size() + elm_global_start_idx;

        for (Uint elm=elm_global_start_idx; elm<elm_global_end_idx; elm++, line_counter++)
        {
          if (line_counter == 10)
          {
            file << std::endl;
            line_counter = 0;
          }
          file << std::setw(8) << elm;
        }
      }
      file << std::endl;
      file << "ENDOFSECTION" << std::endl;
    }
  }
}


//////////////////////////////////////////////////////////////////////////////

void CWriter::write_boundaries(std::fstream& file)
{ 
  // Add node connectivity data at the mesh level
  CNodeConnectivity::Ptr node_connectivity = m_mesh->create_component_type<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(recursive_filtered_range_typed<CElements>(*m_mesh, IsElementsVolume()));

  
  BOOST_FOREACH(CElements& elementregion, recursive_filtered_range_typed<CElements>(*m_mesh,IsElementsSurface()))
    elementregion.create_component_type<CFaceConnectivity>("face_connectivity")->initialize(*node_connectivity);

  // Find total number of boundary elements and store all bc groups
  Uint total_nbElements=0;
  std::set<CRegion::ConstPtr> bc_regions;
  BOOST_FOREACH(const CElements& elementregion, recursive_filtered_range_typed<CElements>(*m_mesh,IsElementsSurface()))
  {
    bc_regions.insert(elementregion.get_parent()->get_type<CRegion const>());
    total_nbElements += elementregion.connectivity_table().size();
  }
  
  if (total_nbElements > 0)
  {
    /// @todo pass a CFLogStream to progress_display instead of std::cout
    boost::progress_display progress(total_nbElements,std::cout,"writing boundary conditions\n");
    
    BOOST_FOREACH(CRegion::ConstPtr group, bc_regions) // For each boundary condition
    {
      file << " BOUNDARY CONDITIONS 2.3.16\n";
      file << std::setw(32) << group->name() << std::setw(8) << 1 << std::setw(8) << group->recursive_elements_count() << std::setw(8) << 0 << std::setw(8) << 6 << std::endl;
      
      BOOST_FOREACH(const CElements& elementregion, recursive_range_typed<CElements>(*group))  // for each element type in this BC
      {
        const CTable<Uint>& table = elementregion.connectivity_table();
        const CFaceConnectivity& face_connectivity = get_component_typed<CFaceConnectivity>(elementregion);
        
        const Uint nb_elems = table.size();
        const Uint nb_faces = elementregion.element_type().nb_faces();
        for(Uint elem = 0; elem != nb_elems; ++elem)
        {
          for(Uint face = 0; face != nb_faces; ++face)
          {
            if(face_connectivity.has_adjacent_element(elem, face))
            {
              CFaceConnectivity::ElementReferenceT connected = face_connectivity.adjacent_element(elem, face);
              
              const CElements* connected_region = connected.first;
              Uint connected_region_start_idx = m_global_start_idx[connected_region];
              
              Uint elm_local_idx = connected.second;
              Uint elm_global_idx = connected_region_start_idx + elm_local_idx;
              Uint neu_elm_type = m_CFelement_to_NeuElement[connected_region->element_type().shape()];
              Uint neu_elm_face_idx = m_faces_cf_to_neu[neu_elm_type][face_connectivity.adjacent_face(elem, face)];
              
              file << std::setw(10) << elm_global_idx+1 << std::setw(5) << neu_elm_type << std::setw(5) << neu_elm_face_idx << std::endl;
              ++progress;
            }
            else
            {
              std::string error_msg = "Face " + String::to_str(face) + " of element " + String::to_str(elem)
                                     + " of " + elementregion.full_path().string() + " has no neighbour."; 
              throw ValueNotFound (FromHere(), error_msg);
            }
          }
        }
      } 
      
      file << "ENDOFSECTION" << std::endl;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Neu
} // Mesh
} // CF
