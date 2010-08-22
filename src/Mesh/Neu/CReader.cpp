#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/filesystem/convenience.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"

#include "Mesh/Neu/CReader.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {

  using namespace Common;
  
////////////////////////////////////////////////////////////////////////////////

CF::Common::ObjectProvider < Mesh::Neu::CReader,
                             Mesh::CMeshReader,
                             Mesh::Neu::NeuLib,
                             1 >
aNeuReader_Provider ( "Neu" );

//////////////////////////////////////////////////////////////////////////////

CReader::CReader( const CName& name )
: CMeshReader(name),
  Shared()
{
  BUILD_COMPONENT;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CReader::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".neu");
  return extensions;
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_from_to(boost::filesystem::path& fp, const CMesh::Ptr& mesh)
{

  // if the file is present open it
  boost::filesystem::fstream file;
  if( boost::filesystem::exists(fp) )
  {
    CFLog(VERBOSE, "Opening file " <<  fp.string() << "\n");
    file.open(fp,std::ios_base::in); // exists so open it
  }
  else // doesnt exist so throw exception
  {
     throw boost::filesystem::filesystem_error( fp.string() + " does not exist",
                                                boost::system::error_code() );
  }

  m_file_basename = boost::filesystem::basename(fp);
  
  // set the internal mesh pointer
  m_mesh = mesh;

  // create a regions component inside the mesh
  m_region = m_mesh->create_region(m_file_basename).get_type<CRegion>();

  // must be in correct order!
  read_headerData(file);
  read_coordinates(file);
  read_connectivity(file);
  read_groups(file);
  read_boundaries(file);


  // Remove tmp region from component
  if (m_headerData.NGRPS != 1)
  {
    remove_component("tmp");
    m_tmp.reset();
  }
  
  // Remove regions with empty connectivity tables
  remove_empty_element_regions(get_component_typed<CRegion>(*m_mesh));

  // truely deallocate this vector
  std::vector<Region_TableIndex_pair>().swap (m_global_to_tmp);

  file.close();
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_headerData(std::fstream& file)
{
  Uint NUMNP, NELEM, NGRPS, NBSETS, NDFCD, NDFVL;
  std::string line;

  // skip 6 initial lines
  for (Uint i=0; i<6; ++i)
    getline(file,line);

  // read number of points, elements, groups, sets, dimensions, velocitycomponents
  getline(file,line);
  std::stringstream ss(line);
  ss >> NUMNP >> NELEM >> NGRPS >> NBSETS >> NDFCD >> NDFVL;

  m_headerData.NUMNP  = NUMNP;
  m_headerData.NELEM  = NELEM;
  m_headerData.NGRPS  = NGRPS;
  m_headerData.NBSETS = NBSETS;
  m_headerData.NDFCD  = NDFCD;
  m_headerData.NDFVL  = NDFVL;
  //m_headerData.print();
  
  getline(file,line);
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_coordinates(std::fstream& file)
{
  // Create the coordinates array
  CF_DEBUG_POINT;
  CArray::Array& coordinates = m_region->create_coordinates(m_headerData.NDFCD).array();
  CF_DEBUG_POINT;
  coordinates.resize(boost::extents[m_headerData.NUMNP][m_headerData.NDFCD]);
  CF_DEBUG_POINT;
  
  std::string line;
  // skip one line
  getline(file,line);

  // declare and allocate one coordinate row
  std::vector<Real> rowVector(m_headerData.NDFCD);

  for (Uint i=0; i<m_headerData.NUMNP; ++i) 
  {
    getline(file,line);
    std::stringstream ss(line);
    Uint nodeNumber;
    ss >> nodeNumber;
    nodeNumber--;
    for (Uint dim=0; dim<m_headerData.NDFCD; ++dim)
      ss >> coordinates[i][dim];
  }
  
  getline(file,line);
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_connectivity(std::fstream& file)
{
  // make temporary regions for each element type possible
  m_tmp = create_component_type<CRegion>("tmp");

  CArray& coordinates = get_component_typed<CArray>(*m_region,IsComponentTag("coordinates"));
  
  std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
      create_element_regions_with_buffermap(*m_tmp,coordinates,m_supported_types);

  // skip next line
  std::string line;
  getline(file,line);
    
  // read every line and store the connectivity in the correct region through the buffer
  std::string etype_CF;
  for (Uint i=0; i<m_headerData.NELEM; ++i) {
    // element description
    Uint elementNumber, elementType, nbElementNodes;
    file >> elementNumber >> elementType >> nbElementNodes;
    elementNumber--;
    // find the element type
    if      (elementType==LINE  && nbElementNodes==2) etype_CF = "Line";  // quadrilateral
    else if (elementType==QUAD  && nbElementNodes==4) etype_CF = "Quad";  // quadrilateral
    else if (elementType==TRIAG && nbElementNodes==3) etype_CF = "Triag"; // triangle
    else if (elementType==HEXA  && nbElementNodes==8) etype_CF = "Hexa";  // brick
    else if (elementType==TETRA && nbElementNodes==4) etype_CF = "Tetra";
    /// @todo to be implemented
    else if (elementType==5 && nbElementNodes==6) // wedge (prism)
      throw Common::NotImplemented(FromHere(),"wedge or prism element not able to convert to COOLFluiD yet.");
    else if (elementType==7 && nbElementNodes==5) // pyramid
      throw Common::NotImplemented(FromHere(),"pyramid element not able to convert to COOLFluiD yet.");
    else {
      throw Common::NotSupported(FromHere(),"no support for element type/nodes "
                                 + StringOps::to_str<int>(elementType) + "/" + StringOps::to_str<int>(nbElementNodes) +
           " in Gambit Neutral format");
    }
    // append dimension to the element type (1D, 2D, 3D)
    etype_CF += StringOps::to_str<int>(m_headerData.NDFCD)+"DLagrangeP1";
    
    // get element nodes
    std::vector<Uint> cf_element(nbElementNodes);
    
    for (Uint j=0; j<nbElementNodes; ++j)
    {
      Uint cf_idx = m_nodes_neu_to_cf[elementType][j];
      file >> cf_element[cf_idx];
      cf_element[cf_idx]--;
    }
    Uint table_idx = buffer[etype_CF]->add_row(cf_element);
    CElements::Ptr tmp_elements = get_named_component_typed_ptr<CElements>(*m_tmp, "elements_" + etype_CF);
    cf_assert(tmp_elements);
    m_global_to_tmp.push_back(Region_TableIndex_pair(tmp_elements,table_idx));
    
    // finish the line
    getline(file,line);
  }
  getline(file,line);  // ENDOFSECTION
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_groups(std::fstream& file)
{
  std::string line;
  int dummy;
  
  CArray& coordinates = get_component_typed<CArray>(*m_region,IsComponentTag("coordinates"));

  std::vector<GroupData> groups(m_headerData.NGRPS);
  for (Uint g=0; g<m_headerData.NGRPS; ++g) {    
    std::string ELMMAT;
    Uint NGP, NELGP, MTYP, NFLAGS, I;
    getline(file,line);  // ELEMENT GROUP...
    file >> line >> NGP >> line >> NELGP >> line >> MTYP >> line >> NFLAGS >> ELMMAT;
    groups[g].NGP    = NGP;
    groups[g].NELGP  = NELGP;
    groups[g].MTYP   = MTYP;
    groups[g].NFLAGS = NFLAGS;
    groups[g].ELMMAT = ELMMAT;
    //groups[g].print();
    
    for (Uint i=0; i<NFLAGS; ++i)
      file >> dummy;
      
    groups[g].ELEM.reserve(NELGP);
    for (Uint i=0; i<NELGP; ++i) 
    {
      file >> I;
      groups[g].ELEM.push_back(I-1);     // set element index
    }
    getline(file,line);  // finish the line (read new line)
    getline(file,line);  // ENDOFSECTION
  }
  
  // 2 cases:
  // 1) there is only one group --> The tmp region can just be renamed
  //    and put in the filesystem as subcomponent of "mesh/regions"
  if (m_headerData.NGRPS == 1)
  {
    m_tmp->rename(groups[0].ELMMAT);
    m_tmp->move_component(m_region);
  }
  // 2) there are multiple groups --> New regions have to be created
  //    and the elements from the tmp region have to be distributed among
  //    these new regions.
  else
  {
    // Create Region for each group
    BOOST_FOREACH(GroupData& group, groups)
    {

      CRegion& region = m_region->create_region(group.ELMMAT);

      // Create regions for each element type in each group-region
      std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
          create_element_regions_with_buffermap(region,coordinates,m_supported_types);

      // Copy elements from tmp_region in the correct region
      BOOST_FOREACH(Uint global_element, group.ELEM)
      {
        CElements::Ptr tmp_region = m_global_to_tmp[global_element].first;
        Uint local_element = m_global_to_tmp[global_element].second;
        boost::shared_ptr<CTable::Buffer> buf = buffer[tmp_region->element_type().getElementTypeName()];
        cf_assert(buf);
        buf->add_row(tmp_region->connectivity_table().table()[local_element]);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_boundaries(std::fstream& file)
{
  CArray& coordinates = get_component_typed<CArray>(*m_region,IsComponentTag("coordinates"));
  
  std::string line;
  for (Uint t=0; t<m_headerData.NBSETS; ++t) {

    std::string NAME;
    int ITYPE, NENTRY, NVALUES, IBCODE1, IBCODE2, IBCODE3, IBCODE4, IBCODE5;

    // read header
    getline(file,line);  // BOUNDARY CONDITIONS...
    getline(file,line);  // header
    std::stringstream ss(line);
    ss >> NAME >> ITYPE >> NENTRY >> NVALUES >> IBCODE1 >> IBCODE2 >> IBCODE3 >> IBCODE4 >> IBCODE5;
    if (ITYPE!=1) {
      throw Common::NotSupported(FromHere(),"error: supports only boundary condition data 1 (element/cell): page C-11 of user's guide");
    }
    if (IBCODE1!=6) {
      throw Common::NotSupported(FromHere(),"error: supports only IBCODE1 6 (ELEMENT_SIDE)");
    }

    // boundary connectivity here
    //vector< GElement > e2n(NENTRY);

    CRegion& bc_region = m_region->create_region(NAME);

    // create all kind of element type regions
    BufferMap buffer = create_element_regions_with_buffermap (bc_region,coordinates,m_supported_types);

    // read boundary elements connectivity
    for (int i=0; i<NENTRY; ++i) {
      int ELEM, ETYPE, FACE;
      file >> ELEM >> ETYPE >> FACE;

      Uint global_element = ELEM-1;
      //Uint elementType = ETYPE;
      Uint faceIdx = m_faces_neu_to_cf[ETYPE][FACE];

      CElements::Ptr tmp_region = m_global_to_tmp[global_element].first;
      Uint local_element = m_global_to_tmp[global_element].second;

      const ElementType& etype = tmp_region->element_type();
      const ElementType::FaceConnectivity& face_connectivity = etype.face_connectivity();
      
      // make a row of nodes
      const CTable::Row& elem_nodes = tmp_region->connectivity_table().table()[local_element];
      std::vector<Uint> row;
      row.reserve(face_connectivity.face_node_counts[faceIdx]);
      BOOST_FOREACH(const Uint& node, face_connectivity.face_node_range(faceIdx))
        row.push_back(elem_nodes[node]);

      // add the row to the buffer of the face region
      buffer[etype.face_type(faceIdx).getElementTypeName()]->add_row(row);

      getline(file,line);  // finish the line (read new line)
    }
    getline(file,line);  // ENDOFSECTION

  }
}


//////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF
