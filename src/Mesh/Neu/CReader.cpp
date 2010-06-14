#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>
#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Mesh/Neu/CReader.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {
  
////////////////////////////////////////////////////////////////////////////////

CF::Common::ObjectProvider < Mesh::Neu::CReader,
                             Mesh::CMeshReader,
                             Mesh::Neu::NeuLib,
                             1 >
aNeuReader_Provider ( "Neu" );

//////////////////////////////////////////////////////////////////////////////

CReader::CReader( const CName& name )
: CMeshReader(name),
  m_faces_cf_to_neu(10),
  m_faces_neu_to_cf(10)
{
  BUILD_COMPONENT;

  m_supported_types.reserve(8);
  m_supported_types.push_back("P1-Line1D");
  m_supported_types.push_back("P1-Line2D");
  m_supported_types.push_back("P1-Line3D");
  m_supported_types.push_back("P1-Quad2D");
  m_supported_types.push_back("P1-Quad3D");
  m_supported_types.push_back("P1-Triag2D");
  m_supported_types.push_back("P1-Triag3D");
  m_supported_types.push_back("P1-Hexa3D");


  // face translation
  enum NeuFace {LINE=1,QUAD=2,TRIAG=3,HEXA=4};

  // line
  m_faces_cf_to_neu[LINE].resize(2);

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

  // set the internal mesh pointer
  m_mesh = mesh;

  // must be in correct order!
  read_headerData(file);
  read_coordinates(file);
  read_connectivity(file);
  read_groups(file);
  read_boundaries(file);
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
  m_mesh->create_array("coordinates");
  // create pointers to the coordinates array
  CArray::Ptr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");
  // set dimension
  coordinates->initialize(m_headerData.NDFCD);
  // create a buffer to interact with coordinates
  CArray::Buffer buffer = coordinates->create_buffer(m_headerData.NUMNP/10);
  
  std::string line;
  // skip one line
  getline(file,line);

  // declare and allocate one coordinate row
  std::vector<Real> rowVector(m_headerData.NDFCD);

  for (Uint i=0; i<m_headerData.NUMNP; ++i) {
    getline(file,line);
    std::stringstream ss(line);
    Uint nodeNumber;
    ss >> nodeNumber;
    nodeNumber--;
    for (Uint dim=0; dim<m_headerData.NDFCD; ++dim)
      ss >> rowVector[dim];
    buffer.add_row(rowVector);
  }
  buffer.flush();
  
  getline(file,line);
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_connectivity(std::fstream& file)
{
  // make temporary regions for each element type possible
  CRegion::Ptr tmp = m_mesh->create_region("tmp");

  std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
      create_leaf_regions_with_buffermap(tmp,m_supported_types);

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
    if      (elementType==1 && nbElementNodes==2) etype_CF = "P1-Line";  // quadrilateral
    else if (elementType==2 && nbElementNodes==4) etype_CF = "P1-Quad";  // quadrilateral
    else if (elementType==3 && nbElementNodes==3) etype_CF = "P1-Triag"; // triangle
    else if (elementType==4 && nbElementNodes==8) etype_CF = "P1-Hexa";  // brick
    /// @todo to be implemented
    else if (elementType==5 && nbElementNodes==6) // wedge (prism)
      throw Common::NotImplemented(FromHere(),"wedge or prism element not able to convert to COOLFluiD yet.");
    else if (elementType==6 && nbElementNodes==4) // tetrahedron
      throw Common::NotImplemented(FromHere(),"tetrahedron element not able to convert to COOLFluiD yet.");
    else if (elementType==7 && nbElementNodes==5) // pyramid
      throw Common::NotImplemented(FromHere(),"pyramid element not able to convert to COOLFluiD yet.");
    else {
      throw Common::NotSupported(FromHere(),"no support for element type/nodes "
                                 + StringOps::to_str<int>(elementType) + "/" + StringOps::to_str<int>(nbElementNodes) +
           " in Gambit Neutral format");
    }
    // append dimension to the element type (1D, 2D, 3D)
    etype_CF += StringOps::to_str<int>(m_headerData.NDFCD)+"D";
    
    // get element nodes
    std::vector<Uint> rowVector(nbElementNodes);
    
    for (Uint j=0; j<nbElementNodes; ++j)
    {
      file >> rowVector[j];
      rowVector[j]--;
    }
    Uint table_idx = buffer[etype_CF]->get_total_nbRows();
    buffer[etype_CF]->add_row(rowVector);
    m_global_to_tmp.push_back(Region_TableIndex_pair(get_named_component_typed_ptr<CRegion>(*tmp, etype_CF),table_idx));
    
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
  
  CRegion::Ptr regions = m_mesh->create_region("regions");
  
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
    Component::Ptr tmp = m_mesh->remove_component("tmp");
    tmp->rename(groups[0].ELMMAT);
    regions->add_component(tmp);
  }
  // 2) there are multiple groups --> New regions have to be created
  //    and the elements from the tmp region have to be distributed among
  //    these new regions.
  else
  {
    // Create Region for each group
    BOOST_FOREACH(GroupData& group, groups)
    {

      CRegion::Ptr region = regions->create_region(group.ELMMAT);

      // Create regions for each element type in each group-region
      std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
          create_leaf_regions_with_buffermap(region,m_supported_types);

      // Copy elements from tmp_region in the correct region
      BOOST_FOREACH(Uint global_element, group.ELEM)
      {
        CRegion::Ptr tmp_region = m_global_to_tmp[global_element].first;
        Uint local_element = m_global_to_tmp[global_element].second;
        buffer[tmp_region->name()]->add_row(get_named_component_typed<CTable>(*tmp_region, "table").get_table()[local_element]);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_boundaries(std::fstream& file)
{
  std::string line;
  CRegion::Ptr regions=get_named_component_typed_ptr<CRegion>(*m_mesh,"regions");
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

    CRegion::Ptr bc_region = regions->create_component_type<CRegion>(NAME);

    // create all kind of element type regions
    BufferMap buffer = create_leaf_regions_with_buffermap (bc_region,m_supported_types);

    // read boundary elements connectivity
    for (int i=0; i<NENTRY; ++i) {
      int ELEM, ETYPE, FACE;
      file >> ELEM >> ETYPE >> FACE;

      Uint global_element = ELEM-1;
      Uint elementType = ETYPE;
      Uint faceIdx = FACE;

      CRegion::Ptr tmp_region = m_global_to_tmp[global_element].first;
      Uint local_element = m_global_to_tmp[global_element].second;

      // translate the Neu face to a CF face
      const ElementType::Face& face = get_component_typed<CElements>(*tmp_region,IsComponentTrue()).getFaces()[m_faces_neu_to_cf[elementType][faceIdx]];

      // make a row of nodes
      const CTable::Row& elem_nodes = get_component_typed<CTable>(*tmp_region,IsComponentTrue()).get_table()[local_element];
      std::vector<Uint> row;
      row.reserve(face.nodes.size());
      BOOST_FOREACH(const Uint& node, face.nodes)
        row.push_back(elem_nodes[node]);

      // add the row to the buffer of the face region
      buffer[face.faceType->getElementTypeName()]->add_row(row);

      getline(file,line);  // finish the line (read new line)
    }
    getline(file,line);  // ENDOFSECTION

  }

  // Remove tmp region from component
  if (m_headerData.NGRPS != 1)
  {
    Component::Ptr tmp = m_mesh->remove_component("tmp");
    tmp.reset();
  }

  // truely deallocate this vector
  std::vector<Region_TableIndex_pair>().swap (m_global_to_tmp);

  // Remove regions with empty connectivity tables
  remove_empty_leaf_regions(regions);

}

//////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF
