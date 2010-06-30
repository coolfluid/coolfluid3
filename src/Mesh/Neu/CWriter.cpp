#include <iostream>
#include <iomanip>

#include <boost/foreach.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/progress.hpp>

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
: CMeshWriter(name),
  m_faces_cf_to_neu(10),
  m_faces_neu_to_cf(10),
  m_nodes_cf_to_neu(10),
  m_nodes_neu_to_cf(10)
{
  BUILD_COMPONENT;


  m_supported_types.reserve(9);
  m_supported_types.push_back("P1-Line1D");
  m_supported_types.push_back("P1-Line2D");
  m_supported_types.push_back("P1-Line3D");
  m_supported_types.push_back("P1-Quad2D");
  m_supported_types.push_back("P1-Quad3D");
  m_supported_types.push_back("P1-Triag2D");
  m_supported_types.push_back("P1-Triag3D");
  m_supported_types.push_back("P1-Hexa3D");
  m_supported_types.push_back("P1-Tetra3D");


  m_CFelement_to_NeuElement[GeoShape::LINE ]=LINE;
  m_CFelement_to_NeuElement[GeoShape::QUAD ]=QUAD;
  m_CFelement_to_NeuElement[GeoShape::TRIAG]=TRIAG;
  m_CFelement_to_NeuElement[GeoShape::HEXA ]=HEXA;
  m_CFelement_to_NeuElement[GeoShape::TETRA]=TETRA;


  // ------------------------------------------------------- FACES
  // line
  m_faces_cf_to_neu[LINE].resize(2);
  m_faces_cf_to_neu[LINE][0]=1;
  m_faces_cf_to_neu[LINE][1]=2;
  
  m_faces_neu_to_cf[LINE].resize(2);
  m_faces_neu_to_cf[LINE][1]=0;
  m_faces_neu_to_cf[LINE][2]=1;
  
  // quad
  m_faces_cf_to_neu[QUAD].resize(4);
  m_faces_cf_to_neu[QUAD][0]=1;
  m_faces_cf_to_neu[QUAD][1]=2;
  m_faces_cf_to_neu[QUAD][2]=3;
  m_faces_cf_to_neu[QUAD][3]=4;
  
  m_faces_neu_to_cf[QUAD].resize(5);
  m_faces_neu_to_cf[QUAD][1]=0;
  m_faces_neu_to_cf[QUAD][2]=1;
  m_faces_neu_to_cf[QUAD][3]=2;
  m_faces_neu_to_cf[QUAD][4]=3;
  
  // triag
  m_faces_cf_to_neu[TRIAG].resize(3);
  m_faces_cf_to_neu[TRIAG][0]=1;
  m_faces_cf_to_neu[TRIAG][1]=2;
  m_faces_cf_to_neu[TRIAG][2]=3;
  
  m_faces_neu_to_cf[TRIAG].resize(4);
  m_faces_neu_to_cf[TRIAG][1]=0;
  m_faces_neu_to_cf[TRIAG][2]=1;
  m_faces_neu_to_cf[TRIAG][3]=2;
  
  // hexa
  m_faces_cf_to_neu[HEXA].resize(6);
  m_faces_cf_to_neu[HEXA][0]=1;
  m_faces_cf_to_neu[HEXA][1]=3;
  m_faces_cf_to_neu[HEXA][2]=6;
  m_faces_cf_to_neu[HEXA][3]=2;
  m_faces_cf_to_neu[HEXA][4]=5;
  m_faces_cf_to_neu[HEXA][5]=4;
  
  m_faces_neu_to_cf[HEXA].resize(7);
  m_faces_neu_to_cf[HEXA][1]=0;
  m_faces_neu_to_cf[HEXA][2]=3;
  m_faces_neu_to_cf[HEXA][3]=1;
  m_faces_neu_to_cf[HEXA][4]=5;
  m_faces_neu_to_cf[HEXA][5]=4;
  m_faces_neu_to_cf[HEXA][6]=2;
  
  // tetra
  m_faces_cf_to_neu[TETRA].resize(4);
  m_faces_cf_to_neu[TETRA][0]=1;
  m_faces_cf_to_neu[TETRA][1]=2;
  m_faces_cf_to_neu[TETRA][2]=3;
  m_faces_cf_to_neu[TETRA][3]=4;
  
  m_faces_neu_to_cf[TETRA].resize(5);
  m_faces_neu_to_cf[TETRA][1]=0;
  m_faces_neu_to_cf[TETRA][2]=1;
  m_faces_neu_to_cf[TETRA][3]=2;
  m_faces_neu_to_cf[TETRA][4]=3;
  
  
  // --------------------------------------------------- NODES
  
  // line
  m_nodes_cf_to_neu[LINE].resize(2);
  m_nodes_cf_to_neu[LINE][0]=0;
  m_nodes_cf_to_neu[LINE][1]=1;
  
  m_nodes_neu_to_cf[LINE].resize(2);
  m_nodes_neu_to_cf[LINE][0]=0;
  m_nodes_neu_to_cf[LINE][1]=1;
  
  // quad
  m_nodes_cf_to_neu[QUAD].resize(4);
  m_nodes_cf_to_neu[QUAD][0]=0;
  m_nodes_cf_to_neu[QUAD][1]=1;
  m_nodes_cf_to_neu[QUAD][2]=2;
  m_nodes_cf_to_neu[QUAD][3]=3;
  
  m_nodes_neu_to_cf[QUAD].resize(4);
  m_nodes_neu_to_cf[QUAD][0]=0;
  m_nodes_neu_to_cf[QUAD][1]=1;
  m_nodes_neu_to_cf[QUAD][2]=2;
  m_nodes_neu_to_cf[QUAD][3]=3;
  
  // triag
  m_nodes_cf_to_neu[TRIAG].resize(3);
  m_nodes_cf_to_neu[TRIAG][0]=0;
  m_nodes_cf_to_neu[TRIAG][1]=1;
  m_nodes_cf_to_neu[TRIAG][2]=2;
  
  m_nodes_neu_to_cf[TRIAG].resize(3);
  m_nodes_neu_to_cf[TRIAG][0]=0;
  m_nodes_neu_to_cf[TRIAG][1]=1;
  m_nodes_neu_to_cf[TRIAG][2]=2;
  
  
  // tetra
  m_nodes_cf_to_neu[TETRA].resize(4);
  m_nodes_cf_to_neu[TETRA][0]=0;
  m_nodes_cf_to_neu[TETRA][1]=1;
  m_nodes_cf_to_neu[TETRA][2]=2;
  m_nodes_cf_to_neu[TETRA][3]=3;
  
  m_nodes_neu_to_cf[TETRA].resize(4);
  m_nodes_neu_to_cf[TETRA][0]=0;
  m_nodes_neu_to_cf[TETRA][1]=1;
  m_nodes_neu_to_cf[TETRA][2]=2;
  m_nodes_neu_to_cf[TETRA][3]=3;
  
  
  // hexa
  m_nodes_cf_to_neu[HEXA].resize(8);
  m_nodes_cf_to_neu[HEXA][0]=4;
  m_nodes_cf_to_neu[HEXA][1]=5;
  m_nodes_cf_to_neu[HEXA][2]=1;
  m_nodes_cf_to_neu[HEXA][3]=0;
  m_nodes_cf_to_neu[HEXA][4]=6;
  m_nodes_cf_to_neu[HEXA][5]=7;
  m_nodes_cf_to_neu[HEXA][6]=3;
  m_nodes_cf_to_neu[HEXA][7]=2;
  
  m_nodes_neu_to_cf[HEXA].resize(8);
  m_nodes_neu_to_cf[HEXA][0]=3;
  m_nodes_neu_to_cf[HEXA][1]=2;
  m_nodes_neu_to_cf[HEXA][2]=7;
  m_nodes_neu_to_cf[HEXA][3]=6;
  m_nodes_neu_to_cf[HEXA][4]=0;
  m_nodes_neu_to_cf[HEXA][5]=1;
  m_nodes_neu_to_cf[HEXA][6]=4;
  m_nodes_neu_to_cf[HEXA][7]=5;

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
  std::string filename="quadtriag"; //without extention

  // get the day of today
  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::gregorian::date::ymd_type today_ymd = today.year_month_day();

  Uint group_counter(0);
  Uint element_counter(0);
  Uint bc_counter(0);
  CArray::Ptr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");
  const Uint coord_dim = coordinates->array().shape()[1];
  Uint node_counter = get_named_component_typed<CArray>(*m_mesh, "coordinates").array().shape()[0];
  Uint max_dimensionality = 0;
  // Find maximal dimensionality of the whole mesh
  BOOST_FOREACH( const CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsElementRegion()))
    max_dimensionality = std::max(region.elements_type().getDimensionality() , max_dimensionality);
  

  BOOST_FOREACH(const CRegion& group, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
  {
    bool isGroupBC(false);
    BOOST_FOREACH(const CRegion& elementregion, recursive_filtered_range_typed<CRegion>(group,IsElementRegion()))
    {
      bool isElementBC(false);
      Uint dimensionality = get_component_typed<CElements>(elementregion,IsComponentTrue()).getDimensionality();
      if (dimensionality < max_dimensionality) // is bc
      {
        isElementBC = true;
        isGroupBC = true;
      }
      if (!isElementBC)
        element_counter += elementregion.elements_count();
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
  CArray::Ptr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");

  const Uint coord_dim = coordinates->array().shape()[1];

  file << "   NODAL COORDINATES 2.3.16" << std::endl;
  file.setf(std::ios::fixed);
  Uint node_number = 0;
  BOOST_FOREACH(const CArray::ConstRow& row, coordinates->array())
  {
    ++node_number;
    file << std::setw(10) << node_number;
    for (Uint d=0; d<coord_dim; ++d)
    {
      //CFinfo << node_number << "(" << d << ") \t " << CFflush;
      file << std::setw(20) << std::scientific << row[d];
    }
    file << std::endl;
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

  // loop over all element regions
  const CArray& coordinates = get_named_component_typed<CArray>(*m_mesh,"coordinates");
  const Uint coord_dim = coordinates.array().shape()[1];
  BOOST_FOREACH(const CRegion& elementregion,recursive_filtered_range_typed<CRegion>(*m_mesh,IsElementRegion()))
  {
    bool isBC = false;
    Uint dimensionality = get_component_typed<CElements>(elementregion,IsComponentTrue()).getDimensionality();
    if (dimensionality < coord_dim) // is bc
    {
      isBC = true;
      //CFinfo << "skip bc region " << elementregion->full_path().string() << CFendl;
    }
    if (!isBC)
    {
      //CFinfo << "elements from region: " << elementregion->full_path().string() << CFendl;
      // information of this region with one unique element type
      Uint elm_type;
      Uint nb_nodes;
      elm_type = m_CFelement_to_NeuElement[get_component_typed<CElements>(elementregion,IsComponentTrue()).getShape()];
      nb_nodes = get_component_typed<CElements>(elementregion,IsComponentTrue()).getNbNodes();
      m_global_start_idx[&elementregion]=elm_number;

      // write the nodes for each element of this region
      BOOST_FOREACH(const CTable::ConstRow& cf_element ,get_component_typed<CTable>(elementregion,IsComponentTrue()).table())
      {
        file << std::setw(8) << ++elm_number << std::setw(3) << elm_type << std::setw(3) << nb_nodes << " ";
        std::vector<Uint> neu_element(nb_nodes);
        
        for (Uint j=0; j<nb_nodes; ++j)
        {
          Uint neu_idx = m_nodes_cf_to_neu[elm_type][j];
          neu_element[neu_idx] = cf_element[j]+1;
        }
        BOOST_FOREACH(Uint neu_node, neu_element)
            file << std::setw(8) << neu_node;
        file << std::endl;
      }
    }
  }
  file << "ENDOFSECTION" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

void CWriter::write_groups(std::fstream& file)
{
  Uint group_counter(0);
  Uint max_dimensionality = 0;
  // Find maximal dimensionality of the whole mesh
  BOOST_FOREACH( const CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsElementRegion()))
    max_dimensionality = std::max(region.elements_type().getDimensionality() , max_dimensionality);
  BOOST_FOREACH(const CRegion& group, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
  {
    bool isBC(false);
    BOOST_FOREACH(const CRegion& elementregion, recursive_filtered_range_typed <CRegion>(group,IsElementRegion()))
    {
      Uint dimensionality = get_component_typed<CElements>(elementregion,IsComponentTrue()).getDimensionality();
      if (dimensionality < max_dimensionality) // is bc
      {
        isBC = true;
      }
    }
    if (!isBC)
    {
      Uint element_counter(0);
      BOOST_FOREACH(const CRegion& elementregion, recursive_filtered_range_typed<CRegion>(group,IsElementRegion()))
      {
        element_counter += elementregion.elements_count();
      }
      file << "       ELEMENT GROUP 2.3.16\n";
      file << "GROUP:" << std::setw(11) << ++group_counter << " ELEMENTS:" << std::setw(11) << element_counter << " MATERIAL:" << std::setw(11) << 2 << " NFLAGS:" << std::setw(11) << 1 << std::endl;
      file << std::setw(32) << group.name() << std::endl << std::setw(8) << 0 << std::endl;
      Uint line_counter=0;
      BOOST_FOREACH(const CRegion& elementregion, recursive_filtered_range_typed <CRegion>(group,IsElementRegion()))
      {
        Uint elm_global_start_idx = m_global_start_idx[&elementregion]+1;
        Uint elm_global_end_idx = elementregion.elements_count() + elm_global_start_idx;

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
  Uint max_dimensionality = 0;
  // Find maximal dimensionality of the whole mesh
  BOOST_FOREACH( const CRegion& region, recursive_filtered_range_typed<CRegion>(*m_mesh,IsElementRegion()))
    max_dimensionality = std::max(region.elements_type().getDimensionality() , max_dimensionality);
  
  create_nodes_to_element_connectivity();

  // Find total number of boundary elements
  Uint total_nbElements=0;
  BOOST_FOREACH(const CRegion& group, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
  {
    BOOST_FOREACH(const CRegion& elementregion, recursive_filtered_range_typed<CRegion>(group,IsElementRegion()))
    {
      Uint dimensionality = get_component_typed<CElements>(elementregion,IsComponentTrue()).getDimensionality();
      if (dimensionality < max_dimensionality) // is bc
      {
        total_nbElements += elementregion.elements_count();
      }
    }
  }

  /// @todo pass a CFLogStream to progress_display instead of std::cout
  boost::progress_display progress(total_nbElements,std::cout,"writing boundary conditions\n");

  BOOST_FOREACH(const CRegion& group, recursive_filtered_range_typed<CRegion>(*m_mesh,IsGroup()))
  {
    bool isBC(false);
    BOOST_FOREACH(const CRegion& elementregion, recursive_filtered_range_typed<CRegion>(group,IsElementRegion()))
    {
      Uint dimensionality = get_component_typed<CElements>(elementregion,IsComponentTrue()).getDimensionality();
      if (dimensionality < max_dimensionality) // is bc
      {
        isBC = true;
      }
    }
    if (isBC)
    {
      Uint element_counter=0;
      BOOST_FOREACH(const CRegion& elementregion, recursive_filtered_range_typed<CRegion>(group,IsElementRegion()))
      {
        element_counter += elementregion.elements_count();
      }
      file << " BOUNDARY CONDITIONS 2.3.16\n";
      file << std::setw(32) << group.name() << std::setw(8) << 1 << std::setw(8) << element_counter << std::setw(8) << 0 << std::setw(8) << 6 << std::endl;

      BOOST_FOREACH(const CRegion& elementregion, recursive_filtered_range_typed<CRegion>(group,IsElementRegion()))
      {
        const CElements& faceType = get_component_typed<CElements>(elementregion,IsComponentTrue());
        const CTable& table = get_component_typed<CTable>(elementregion,IsComponentTrue());
        BOOST_FOREACH(CTable::ConstRow face_nodes, table.table())
        {
          const CRegion* elm_region;
          Uint elm_local_idx;
          Uint elm_face_idx;
          boost::tie(elm_region,elm_local_idx,elm_face_idx) =
              find_element_for_face(faceType,face_nodes,*m_mesh);
          Uint elementregion_start_idx = m_global_start_idx[elm_region];
          Uint elm_global_idx = elementregion_start_idx + elm_local_idx;
          Uint neu_elm_type = m_CFelement_to_NeuElement[get_component_typed<CElements>(*elm_region,IsComponentTrue()).getShape()];
          Uint neu_elm_face_idx = m_faces_cf_to_neu[neu_elm_type][elm_face_idx];

          file << std::setw(10) << elm_global_idx+1 << std::setw(5) << neu_elm_type << std::setw(5) << neu_elm_face_idx << std::endl;
          ++progress;
        }
      }
      file << "ENDOFSECTION" << std::endl;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


void CWriter::create_nodes_to_element_connectivity()
{
  BOOST_FOREACH(const CRegion& elementregion, recursive_filtered_range_typed<CRegion>(*m_mesh,IsElementRegion()))
  {
    const CTable& elements = get_component_typed<CTable>(elementregion,IsComponentTrue());
    Uint elem_idx=0;
    BOOST_FOREACH(const CTable::ConstRow& elem, elements.table())
    {
      BOOST_FOREACH(const Uint node, elem)
      {
        m_n2e[node].push_back(std::make_pair(&elementregion,elem_idx));
      }
      ++elem_idx;
    }
  }

//  typedef std::map<Uint,std::vector<std::pair<CRegion::Ptr,Uint> > >::value_type node_t;
//  typedef std::pair<CRegion::Ptr,Uint> elem_t;
//  BOOST_FOREACH(node_t node, m_n2e)
//  {
//    CFinfo << std::setw(8) << node.first << " --> ";
//    BOOST_FOREACH(elem_t elem, node.second)
//    {
//      CFinfo << std::setw(12) << elem.first->get_parent()->name() << "(" <<elem.second<<") ";
//    }
//    CFinfo << CFendl;
//  }
}
//////////////////////////////////////////////////////////////////////////////

boost::tuple<CRegion const* const,Uint,Uint> CWriter::find_element_for_face(const CElements& face, const CTable::ConstRow& nodes, const Component& parent)
{

  std::vector<Uint> sorted_nodes;
  BOOST_FOREACH(Uint node,nodes)
    sorted_nodes.push_back(node);
  std::sort(sorted_nodes.begin(), sorted_nodes.end());
//  CFinfo << "look for ";
//  BOOST_FOREACH(Uint sorted_node, sorted_nodes)
//    CFinfo << sorted_node << " ";
//  CFinfo << CFendl;

  std::vector<Uint> sorted_face_row(sorted_nodes.size());
  typedef std::pair<CRegion const* const,Uint> elem_t;
  BOOST_FOREACH(elem_t elem, m_n2e[sorted_nodes[0]])
  {
    Uint face_idx=0;
    BOOST_FOREACH(const ElementType::Face& look_face, get_component_typed<CElements>((*elem.first),IsComponentTrue()).getFaces())
    {
//      CFinfo << "    check " << elem.first->full_path().string() << "   row " << elem.second << CFflush;
//      CFinfo << "   face " << face_idx << "  " << CFflush;

      // first check if the face type matches
      if (look_face.faceType->getShape() == face.getShape() &&
          look_face.faceType->getNbNodes() == face.getNbNodes())
      {
        const CTable::ConstRow& elemNodes = get_component_typed<CTable>((*elem.first),IsComponentTrue()).table()[elem.second];

//        CFinfo << "  idx " << CFflush;
//        BOOST_FOREACH(Uint idx, look_face.nodes)
//            CFinfo << idx << " " << CFflush;
//        CFinfo << " of nodes " << CFflush;
//        BOOST_FOREACH(Uint node, elemNodes)
//              CFinfo << node << " ";
//        CFinfo << CFendl;

        for (Uint i=0; i<sorted_nodes.size(); ++i)
          sorted_face_row[i]=elemNodes[look_face.nodes[i]];
        std::sort(sorted_face_row.begin(), sorted_face_row.end());
        //BOOST_FOREACH(Uint sorted_node, sorted_face_row)
        //  CFinfo << sorted_node << " ";
        //CFinfo << CFendl;
        Uint counter=0;
        BOOST_FOREACH(Uint sorted_row_elem, sorted_face_row)
        {
          if (sorted_row_elem == sorted_nodes[counter])
          {
            ++counter;
            if (counter == sorted_nodes.size())
            {
              //CFinfo << " FOUND!!! in " << elementregion->full_path().string()
              //       << " element " << row_idx
              //       << " face " << face_idx << CFendl;
              return boost::make_tuple(elem.first,elem.second,face_idx);
            }
          }
          else
            break;
        }
      }
      ++face_idx;
    }
  }
  throw Common::ShouldNotBeHere(FromHere(),"no element found for node");
  CRegion* nullptr(NULL);
  return boost::make_tuple(nullptr ,0,0);
}

//////////////////////////////////////////////////////////////////////////////


} // Neu
} // Mesh
} // CF
