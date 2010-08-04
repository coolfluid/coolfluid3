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
  m_faces_cf_to_neu(10),
  m_faces_neu_to_cf(10),
  m_nodes_cf_to_neu(10),
  m_nodes_neu_to_cf(10)
{
  BUILD_COMPONENT;

  m_supported_types.reserve(9);
  m_supported_types.push_back("Line1DLagrangeP1");
  m_supported_types.push_back("Line2DLagrangeP1");
  m_supported_types.push_back("Line3DLagrangeP1");
  m_supported_types.push_back("Quad2DLagrangeP1");
  m_supported_types.push_back("Quad3DLagrangeP1");
  m_supported_types.push_back("Triag2DLagrangeP1");
  m_supported_types.push_back("Triag3DLagrangeP1");
  m_supported_types.push_back("Hexa3DLagrangeP1");
  m_supported_types.push_back("Tetra3DLagrangeP1");

  
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

  // set the internal mesh pointer
  m_mesh = mesh;

  // must be in correct order!
  read_headerData(file);
  read_coordinates(file);
  read_connectivity(file);
  read_groups(file);
  read_boundaries(file);


  // Remove regions with empty connectivity tables
  remove_empty_element_regions(get_named_component_typed<CRegion>(*m_mesh,"regions"));

  // Remove tmp region from component
  if (m_headerData.NGRPS != 1)
  {
    // CFinfo << "removing tmp region" <<CFendl;
    Component::Ptr tmp = m_mesh->remove_component("tmp");
    tmp.reset();
  }


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
  m_mesh->create_array("coordinates");
  // create pointers to the coordinates array
  CArray::Ptr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");
  // set dimension
  coordinates->initialize(m_headerData.NDFCD);
  // create a buffer to interact with coordinates
  CArray::Buffer buffer = coordinates->create_buffer();
  buffer.increase_array_size(m_headerData.NUMNP);
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
    buffer.add_row_directly(rowVector);
  }
  buffer.flush();
  
  getline(file,line);
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_connectivity(std::fstream& file)
{
  // make temporary regions for each element type possible
  CRegion::Ptr tmp = m_mesh->create_region("tmp");

  CArray::ConstPtr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");
  
  std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
      create_element_regions_with_buffermap(*tmp,coordinates,m_supported_types);

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
    CElements::Ptr tmp_elements = get_named_component_typed_ptr<CElements>(*tmp, "elements_" + etype_CF);
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
  
  CRegion::Ptr regions = m_mesh->create_region("regions");
  CArray::ConstPtr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");

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

      CRegion& region = regions->create_region(group.ELMMAT);

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
  CArray::ConstPtr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");
  
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

    CRegion& bc_region = regions->create_region(NAME);

    // create all kind of element type regions
    BufferMap buffer = create_element_regions_with_buffermap (bc_region,coordinates,m_supported_types);

    // read boundary elements connectivity
    for (int i=0; i<NENTRY; ++i) {
      int ELEM, ETYPE, FACE;
      file >> ELEM >> ETYPE >> FACE;

      Uint global_element = ELEM-1;
      //Uint elementType = ETYPE;
      Uint faceIdx = FACE-1;

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
