// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>
#include <iomanip>

#include <boost/date_time/gregorian/gregorian.hpp> //include all types plus i/o
#include <boost/progress.hpp>

#include "common/BoostFilesystem.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/StringConversion.hpp"

#include "mesh/neu/Writer.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/ConnectivityData.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/MeshMetadata.hpp"
#include "mesh/Connectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace neu {

using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < mesh::neu::Writer,
                           mesh::MeshWriter,
                           mesh::neu::LibNeu>
aneuWriter_Builder;

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
  extensions.push_back(".neu");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void Writer::write()
{
  // if the file is present open it
  boost::filesystem::fstream file;
  boost::filesystem::path path(m_file_path.path());
  CFinfo <<  "Opening file " <<  path.string() << CFendl;
  file.open(path,std::ios_base::out);
  if (!file) // didn't open so throw exception
  {
     throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                                boost::system::error_code() );
  }
  m_fileBasename = boost::filesystem::basename(path);

  // must be in correct order!
  write_headerData(file, *m_mesh);
  write_coordinates(file, *m_mesh);
  write_connectivity(file, *m_mesh);
  write_groups(file, *m_mesh);

  try
  {
    write_boundaries(file, *m_mesh);
  }
  catch( ... )
  {
  }

  file.close();
}

/////////////////////////////////////////////////////////////////////////////

void Writer::write_headerData(std::fstream& file, const Mesh& mesh)
{
  // get the day of today
  boost::gregorian::date date = boost::gregorian::from_simple_string(mesh.metadata().properties().value_str("date"));

  Uint group_counter(0);
  Uint element_counter(0);
  Uint bc_counter(0);

  const Uint node_counter = mesh.geometry_fields().size();


  boost_foreach(const Region& group, find_components_recursively_with_filter<Region>(mesh,m_region_filter))
  {
    bool isGroupBC(false);
    boost_foreach(const Elements& elementregion, find_components_recursively<Elements>(group))
    {
      bool isElementBC(false);
      Uint dimensionality = elementregion.element_type().dimensionality();
      if (dimensionality < mesh.dimensionality()) // is bc
      {
        isElementBC = true;
        isGroupBC = true;
      }
      if (!isElementBC)
        element_counter += elementregion.geometry_space().connectivity().size();
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
  file << std::setw(4)  << std::string(date.month().as_long_string()).substr(0,3) << " " << int(date.year()) << "\n";
  file << std::setw(10) << "NUMNP" << std::setw(10) << "NELEM" << std::setw(10) << "NGRPS"
       << std::setw(10) << "NBSETS" << std::setw(10) << "NDFCD" << std::setw(10) << "NDFVL" << std::endl;
  file << std::setw(10) << node_counter << std::setw(10) << element_counter << std::setw(10) << group_counter
       << std::setw(10) << bc_counter << std::setw(10) << mesh.dimension() << std::setw(10) << mesh.dimension() << std::endl;
  file << "ENDOFSECTION" << std::endl ;
}

//////////////////////////////////////////////////////////////////////////////

void Writer::write_coordinates(std::fstream& file, const Mesh& mesh)
{
  // set precision for Real
  Uint prec = file.precision();
  file.precision(11);


  file << "   NODAL COORDINATES 2.3.16" << std::endl;
  file.setf(std::ios::fixed);
  Uint node_number = 0;
  boost_foreach(common::Table<Real>::ConstRow row, mesh.geometry_fields().coordinates().array())
  {
    ++node_number;
    file << std::setw(10) << node_number;
    for (Uint d=0; d<mesh.dimension(); ++d)
      file << std::setw(20) << std::scientific << row[d];
    file << std::endl;
  }
  file << "ENDOFSECTION" << std::endl;
  // restore precision
  file.precision(prec);
}

//////////////////////////////////////////////////////////////////////////////

void Writer::write_connectivity(std::fstream& file, const Mesh& mesh)
{
  file << "      ELEMENTS/CELLS 2.3.16" << std::endl;

  // global element number
  Uint elm_number=0;

  // loop over all element regions
  Uint node_idx=0;
  boost_foreach(const Elements& elementregion, find_components_recursively<Elements>(mesh.topology()))
  {
    bool isBC = false;
    Uint dimensionality = elementregion.element_type().dimensionality();
    if (dimensionality < mesh.dimensionality()) // is bc
    {
      isBC = true;
    }
    if (!isBC)
    {
      //CFinfo << "elements from region: " << elementregion->uri().string() << CFendl;
      // information of this region with one unique element type
      Uint elm_type;
      Uint nb_nodes;
      elm_type = m_CFelement_to_neuElement[elementregion.element_type().shape()];
      nb_nodes = elementregion.element_type().nb_nodes();
      m_global_start_idx[Handle<Elements const>(elementregion.handle<Component>())]=elm_number;

      // write the nodes for each element of this region
      boost_foreach(const Connectivity::ConstRow& cf_element , elementregion.geometry_space().connectivity().array())
      {
        file << std::setw(8) << ++elm_number << std::setw(3) << elm_type << std::setw(3) << nb_nodes << " ";
        std::vector<Uint> neu_element(nb_nodes);

        // fill the neu_element (connectivity)
        for (Uint j=0; j<nb_nodes; ++j)
        {
          // index within a neu element (because of different node numbering)
          Uint neu_idx = m_nodes_cf_to_neu[elm_type][j];
          // put the global element number inside the row
          neu_element[neu_idx] = node_idx+cf_element[j]+1;
        }

        Uint eol_counter=0;
        boost_foreach(Uint neu_node, neu_element)
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
  file << "ENDOFSECTION" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

void Writer::write_groups(std::fstream& file, const Mesh& mesh)
{
  Uint group_counter(0);

  boost_foreach(const Region& group, find_components_recursively_with_filter<Region>(mesh,m_region_filter))
  {
    bool isBC(false);
    boost_foreach(const Elements& elementregion, find_components_recursively <Elements>(group))
    {
      Uint dimensionality = elementregion.element_type().dimensionality();
      if (dimensionality < mesh.dimensionality()) // is bc
      {
        isBC = true;
      }
    }
    if (!isBC)
    {
      Uint element_counter(0);
      boost_foreach(const Elements& elementregion, find_components_recursively<Elements>(group))
      {
        element_counter += elementregion.geometry_space().connectivity().size();
      }
      file << "       ELEMENT GROUP 2.3.16\n";
      file << "GROUP:" << std::setw(11) << ++group_counter << " ELEMENTS:" << std::setw(11) << element_counter << " MATERIAL:" << std::setw(11) << 2 << " NFLAGS:" << std::setw(11) << 1 << std::endl;
      file << std::setw(32) << group.name() << std::endl << std::setw(8) << 0 << std::endl;
      Uint line_counter=0;
      boost_foreach(const Elements& elementregion, find_components_recursively <Elements>(group))
      {
        Uint elm_global_start_idx = m_global_start_idx[Handle<Elements const>(elementregion.handle<Component>())]+1;
        Uint elm_global_end_idx = elementregion.geometry_space().connectivity().size() + elm_global_start_idx;

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

void Writer::write_boundaries(std::fstream& file, const Mesh& mesh)
{
  // Add node connectivity data at the mesh level
  Handle<CNodeConnectivity> node_connectivity = create_component<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(find_components_recursively_with_filter<Elements>(*Handle<Mesh const>(mesh.handle<Component>()), IsElementsVolume()));

  std::map<Handle< Elements const >,boost::shared_ptr< CFaceConnectivity > > element_2_face_connecitivity;
  boost_foreach(const Elements& elementregion, find_components_recursively_with_filter<Elements>(mesh,IsElementsSurface()))
  {
    element_2_face_connecitivity[Handle<Elements const>(elementregion.handle<Component>())] = allocate_component<CFaceConnectivity>("face_connectivity");
    element_2_face_connecitivity[Handle<Elements const>(elementregion.handle<Component>())]->initialize(elementregion,*node_connectivity);
  }

  // Find total number of boundary elements and store all bc groups
  Uint total_nbElements=0;
  std::set<Handle< Region const > > bc_regions;
  boost_foreach(const Elements& elementregion, find_components_recursively_with_filter<Elements>(mesh,IsElementsSurface()))
  {
    bc_regions.insert(Handle<Region const>(elementregion.parent()));
    total_nbElements += elementregion.geometry_space().connectivity().size();
  }

  if (total_nbElements > 0)
  {
    /// @todo pass a CFLogStream to progress_display instead of std::cout
    boost::progress_display progress(total_nbElements,std::cout,"writing boundary conditions\n");

    boost_foreach(Handle< Region const > group, bc_regions) // For each boundary condition
    {
      file << " BOUNDARY CONDITIONS 2.3.16\n";
      file << std::setw(32) << group->name() << std::setw(8) << 1 << std::setw(8) << group->recursive_elements_count(m_enable_overlap) << std::setw(8) << 0 << std::setw(8) << 6 << std::endl;

      boost_foreach(const Elements& elementregion, find_components_recursively<Elements>(*group))  // for each element type in this BC
      {
        const Connectivity& table = elementregion.geometry_space().connectivity();
        const CFaceConnectivity& face_connectivity = *element_2_face_connecitivity[Handle<Elements const>(elementregion.handle<Component>())];

        const Uint nb_elems = table.size();
        const Uint nb_faces = elementregion.element_type().nb_faces();
        for(Uint elem = 0; elem != nb_elems; ++elem)
        {
          for(Uint face = 0; face != nb_faces; ++face)
          {
            if(face_connectivity.has_adjacent_element(elem, face))
            {
              CFaceConnectivity::ElementReferenceT connected = face_connectivity.adjacent_element(elem, face);

              Handle< Elements const> connected_region = Handle<Elements const>(connected.first->handle<Component>());
              Uint connected_region_start_idx = m_global_start_idx[connected_region];

              Uint elm_local_idx = connected.second;
              Uint elm_global_idx = connected_region_start_idx + elm_local_idx;
              Uint neu_elm_type = m_CFelement_to_neuElement[connected_region->element_type().shape()];
              Uint neu_elm_face_idx = m_faces_cf_to_neu[neu_elm_type][face_connectivity.adjacent_face(elem, face)];

              file << std::setw(10) << elm_global_idx+1 << std::setw(5) << neu_elm_type << std::setw(5) << neu_elm_face_idx << std::endl;
              ++progress;
            }
            else
            {
              std::stringstream error_msg;
              error_msg << "\nFace " << face << " of element " << elem
                        << " of " << elementregion.uri().string() << " has no neighbour." << std::endl;
              error_msg << "nb_elems = " << nb_elems << std::endl;
              error_msg << "nb_faces per elem = " << nb_faces << std::endl;
              error_msg << "elem coordinates:\n";
              for( Uint n=0; n<elementregion.element_type().nb_nodes(); ++n)
              {
                error_msg << elementregion.geometry_space().connectivity()[elem][n] << ": ";
                error_msg << elementregion.geometry_space().get_coordinates(elem).row(n) << std::endl;
              }
              boost_foreach( Uint n, elementregion.element_type().faces().nodes_range(face) )
              {
                error_msg << "node " << elementregion.geometry_space().connectivity()[elem][n] << "   face coord: " << elementregion.geometry_space().get_coordinates(elem).row(n) << std::endl;
                error_msg << "     connected elems: \n";
                boost_foreach( Uint nc_elem_idx, node_connectivity->node_element_range( elementregion.geometry_space().connectivity()[elem][n] ) )
                {
                  std::pair<Elements const*, Uint> element = node_connectivity->element(nc_elem_idx);
                  error_msg << "          " << element.first->uri() << "[" << element.second << "]  ";
                  error_msg << "   ( ";
                  boost_foreach ( Uint elem_n, element.first->geometry_space().connectivity()[element.second] )
                  {
                     error_msg << elem_n << " ";
                  }
                  error_msg << ")" << std::endl;
                }

              }
              error_msg << "It could be that faces are oriented in the wrong way!!!" << std::endl;
              throw ValueNotFound (FromHere(), error_msg.str());
            }
          }
        }
      }

      file << "ENDOFSECTION" << std::endl;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // neu
} // mesh
} // cf3
