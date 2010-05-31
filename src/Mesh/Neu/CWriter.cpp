#include <boost/foreach.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <iostream>
#include <iomanip>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/Neu/CWriter.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Mesh::Neu::CWriter,
                         Mesh::CMeshWriter,
                         Mesh::Neu::NeuLib,
                         1 >
aNeuWriter_Provider ( "Neu" );

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const CName& name )
: CMeshWriter(name)
{
  build_component(this);

  m_supported_types.reserve(2);
  m_supported_types.push_back("P1-Quad2D");
  m_supported_types.push_back("P1-Triag2D");
  m_supported_types.push_back("P1-Hexa3D");

  m_CFelement_to_NeuElement[GeoShape::QUAD]=2;
  m_CFelement_to_NeuElement[GeoShape::TRIAG]=3;
  m_CFelement_to_NeuElement[GeoShape::HEXA]=4;
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
  write_headerData(file);
  write_coordinates(file);
  write_connectivity(file);
  write_groups(file);

  file.close();

}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_headerData(std::fstream& file)
{
  std::string filename="quadtriag"; //without extention

  // get the day of today
  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::gregorian::date::ymd_type today_ymd = today.year_month_day();

//  CFinfo << "removing empty regions" << CFendl;
//  // loop over regions
//  BOOST_FOREACH(const Component::Ptr& region, iterate_recursive(m_mesh,IsLeafRegion()))
//  {
//    CFinfo << region->full_path().string() << " size : " << region->get_component<CTable>("table")->get_table().size() << CFendl;
//    // find the empty regions
//    if ( region->get_unique_component_by_type<CTable>()->get_table().size() == 0 )
//      {
//        // no elements in connectivity table --> remove this region
//        CFinfo << "remove: " << region->full_path().string() << "\n" << CFflush;
//        region->get_parent()->remove_component(region->name());
//      }
//  }

  Uint group_counter(0);
  Uint element_counter(0);
  Uint bc_counter(0);
  Uint node_counter = m_mesh->get_component<CArray>("coordinates")->get_array().shape()[0];
  Uint coord_dim    = m_mesh->get_component<CArray>("coordinates")->get_array().shape()[1];

  BOOST_FOREACH(const CRegion::Ptr& group, iterate_recursive_by_type<CRegion>(*m_mesh,IsGroup()))
  {
    group_counter++;
  }
  BOOST_FOREACH(const CRegion::Ptr& leafregion, iterate_recursive_by_type<CRegion>(*m_mesh,IsLeafRegion()))
  {
    element_counter += leafregion->getNbElements();
  }
  BOOST_FOREACH(const Component::Ptr& bc_region, iterate_recursive(m_mesh,IsComponentTag("bc")))
  {
    bc_counter++;
  }

  file.setf(std::ios::right);
  CFinfo << "group_counter = " << group_counter << CFendl;
  CFinfo << "element_counter = " << element_counter << CFendl;
  CFinfo << "node_counter = " << node_counter << CFendl;
  file << "        CONTROL INFO 2.3.16\n";
  file << "** GAMBIT NEUTRAL FILE\n";
  file << filename << "\n";
  file << "PROGRAM:                Gambit     VERSION:  2.3.16\n";
  file << today_ymd.month.as_long_string() << " " << today_ymd.year << "\n";
  file << std::setw(10) << "NUMNP" << std::setw(10) << "NELEM" << std::setw(10) << "NGRPS"
       << std::setw(10) << "NBSETS" << std::setw(10) << "NDFCD" << std::setw(10) << "NDFVL" << std::endl;
  file << std::setw(10) << node_counter << std::setw(10) << element_counter << std::setw(10) << group_counter
       << std::setw(10) << bc_counter << std::setw(10) << coord_dim << std::setw(10) << coord_dim << std::endl;
  file << "ENDOFSECTION" << std::endl ;
}

//////////////////////////////////////////////////////////////////////////////

void CWriter::write_coordinates(std::fstream& file)
{
  // set precision for Real
  Uint prec = file.precision();
  file.precision(11);
  CArray::Ptr coordinates = m_mesh->get_component<CArray>("coordinates");

  const Uint coord_dim = coordinates->get_array().shape()[1];

  file << "   NODAL COORDINATES 2.3.16\n";
  file.setf(std::ios::fixed);
  Uint node_number = 0;
  BOOST_FOREACH(CArray::Row row, coordinates->get_array())
  {
    node_number++;
    file << std::setw(10) << node_number;
    for (Uint d=0; d<coord_dim; d++)
      file << std::setw(20) << std::scientific << row[d];
    file << "\n";
  }
  file << "ENDOFSECTION" << std::endl;
  // restore precision
  file.precision(prec);
}

//////////////////////////////////////////////////////////////////////////////

void CWriter::write_connectivity(std::fstream& file)
{
  file << "      ELEMENTS/CELLS 2.3.16" << std::endl;

  // global element number
  Uint elm_number=0;

  // loop over all leaf regions
  BOOST_FOREACH(const CRegion::Ptr& leafregion, iterate_recursive_by_type<CRegion>(*m_mesh,IsLeafRegion()))
  {
    // information of this region with one unique element type
    Uint elm_type;
    Uint nb_nodes;
    elm_type = m_CFelement_to_NeuElement[leafregion->get_unique_component_by_type<CElements>()->getShape()];
    nb_nodes = leafregion->get_unique_component_by_type<CElements>()->getNbNodes();
    m_global_start_idx[leafregion]=elm_number;

    // write the nodes for each element of this region
    BOOST_FOREACH(CTable::Row row, leafregion->get_unique_component_by_type<CTable>()->get_table())
    {
      file << std::setw(8) << ++elm_number << std::setw(3) << elm_type << std::setw(3) << nb_nodes;
      BOOST_FOREACH(Uint node, row)
          file << std::setw(9) << node+1;
      file << std::endl;
    }
  }
  file << "ENDOFSECTION" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

void CWriter::write_groups(std::fstream& file)
{
  Uint group_counter(0);
  BOOST_FOREACH(const CRegion::Ptr& group, iterate_recursive_by_type<CRegion>(*m_mesh,IsGroup()))
  {
    Uint element_counter(0);
    BOOST_FOREACH(const CRegion::Ptr& leafregion, iterate_recursive_by_type<CRegion>(*group,IsLeafRegion()))
    {
      element_counter += leafregion->getNbElements();
    }
    file << "       ELEMENT GROUP 2.3.16\n";
    file << "GROUP:        " << ++group_counter << "  ELEMENTS:         " << element_counter << "  MATERIAL:          2" << " NFLAGS:         1\n";
    file << std::setw(32) << group->name() << std::endl << std::setw(8) << 0 << std::endl;
    Uint line_counter=0;
    BOOST_FOREACH(const CRegion::Ptr& leafregion, iterate_recursive_by_type<CRegion>(*group,IsLeafRegion()))
    {
      Uint elm_global_start_idx = m_global_start_idx[leafregion]+1;
      Uint elm_global_end_idx = leafregion->getNbElements() + elm_global_start_idx;

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


//////////////////////////////////////////////////////////////////////////////

void CWriter::write_boundaries(std::fstream& file)
{
  Uint group_counter(0);
  BOOST_FOREACH(const CRegion::Ptr& group, iterate_recursive_by_type<CRegion>(*m_mesh,IsComponentTag("bc")))
  {
    Uint element_counter(0);
    BOOST_FOREACH(const CRegion::Ptr& leafregion, iterate_recursive_by_type<CRegion>(*group,IsLeafRegion()))
    {
      element_counter += leafregion->getNbElements();
    }
    file << " BOUNDARY CONDITIONS 2.3.16\n";
    file << std::setw(32) << group->name() << std::setw(10) << 1 << std::setw(10) << element_counter << std::setw(10) << 0 << std::setw(10) << 6 << std::endl;
//    Uint line_counter=0;
//    BOOST_FOREACH(const CRegion::Ptr& leafregion, iterate_recursive_by_type<CRegion>(group,IsLeafRegion()))
//    {
//      Uint elm_global_start_idx = m_global_start_idx[leafregion]+1;
//      Uint elm_global_end_idx = leafregion->getNbElements() + elm_global_start_idx;

//      for (Uint elm=elm_global_start_idx; elm<elm_global_end_idx; elm++, line_counter++)
//      {
//        if (line_counter == 10)
//        {
//          file << "\n";
//          line_counter = 0;
//        }
//        file << "\t" << elm;
//      }
//    }
//    file << "\n";
    file << "ENDOFSECTION" << std::endl;
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF
