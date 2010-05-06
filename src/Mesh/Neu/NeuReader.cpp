#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Mesh/Neu/NeuReader.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < NeuReader,
                         Mesh::MeshReader,
                         NeuLib >
aNeuReader_Provider ( "NeuReader" );

//////////////////////////////////////////////////////////////////////////////

NeuReader::NeuReader()
: MeshReader()
{
  m_supported_types.reserve(2);
  m_supported_types.push_back("Quad2D");
  m_supported_types.push_back("Triag2D");
}

//////////////////////////////////////////////////////////////////////////////

void NeuReader::read_headerData(std::fstream& file)
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
  
  getline(file,line);
  cf_assert(line.compare("ENDOFSECTION"));
}

//////////////////////////////////////////////////////////////////////////////

void NeuReader::read_coordinates(std::fstream& file)
{
  // Create the coordinates array
  m_mesh->create_array("coordinates");
  // create pointers to the coordinates array
  boost::shared_ptr<CArray> coordinates = m_mesh->get_component<CArray>("coordinates");
  // set dimension
  coordinates->initialize(m_headerData.NDFCD);
  // create a buffer to interact with coordinates
  CArray::Buffer buffer = coordinates->create_buffer(m_headerData.NUMNP);
  
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
  cf_assert(line.compare("ENDOFSECTION"));
}

//////////////////////////////////////////////////////////////////////////////

void NeuReader::read_connectivity(std::fstream& file)
{
  // make temporary regions for each element type possible
  m_mesh->create_region("tmp");
  boost::shared_ptr<CRegion> tmp = m_mesh->get_component<CRegion>("tmp");
  boost::shared_ptr<CTable::Buffer> buffer;
  boost::shared_ptr<Uint> elementCounter;
  boost::shared_ptr<CRegion> region;

  // quadrilateral
  tmp->create_region_with_elementType("Quad2D");
  boost::shared_ptr<CTable::Buffer> quad2D_buffer 
      (new CTable::Buffer(tmp->get_component<CRegion>("Quad2D")->get_component<CTable>("table")->create_buffer()));
  boost::shared_ptr<Uint> quad2D_elementCounter (new Uint(0));
  boost::shared_ptr<CRegion> quad2D_region = tmp->get_component<CRegion>("Quad2D");

  // triangle
  tmp->create_region_with_elementType("Triag2D");
  boost::shared_ptr<CTable::Buffer> triag2D_buffer 
      (new CTable::Buffer(tmp->get_component<CRegion>("Triag2D")->get_component<CTable>("table")->create_buffer()));
  boost::shared_ptr<Uint> triag2D_elementCounter (new Uint(0));
  boost::shared_ptr<CRegion> triag2D_region = tmp->get_component<CRegion>("Triag2D");

  
  
  // skip next line
  std::string line;
  getline(file,line);
    
  // read every line and store the connectivity in the correct region through the buffer
  for (Uint i=0; i<m_headerData.NELEM; ++i) {
    // element description
    Uint elementNumber, elementType, nbElementNodes;
    file >> elementNumber >> elementType >> nbElementNodes;
    elementNumber--;
    if      (elementType==2 && nbElementNodes==4) // quadrilateral
    {
      buffer         = quad2D_buffer; 
      region         = quad2D_region;
      elementCounter = quad2D_elementCounter;
    }
    else if (elementType==3 && nbElementNodes==3) // triangle
    {
      buffer         = triag2D_buffer; 
      region         = triag2D_region;
      elementCounter = triag2D_elementCounter;
    }
    /// @todo to be implemented
    // else if (elementType==4 && nbElementNodes==8) ;// brick
    // else if (elementType==5 && nbElementNodes==6) ;// wedge (prism)
    // else if (elementType==6 && nbElementNodes==4) ;// tetrahedron
    // else if (elementType==7 && nbElementNodes==5) ;// pyramid
    else {
      CFerr << "error: no support for element type/nodes " << elementType << "/" << nbElementNodes << CFendl;
    }
    
    // get element nodes
    std::vector<Uint> rowVector(nbElementNodes);
    
    for (Uint j=0; j<nbElementNodes; ++j)
    {
      file >> rowVector[j];
      rowVector[j]--;
    }
    buffer->add_row(rowVector);
    m_global_to_tmp.push_back(Region_TableIndex_pair(region,(*elementCounter)++));
    
    // finish the line
    getline(file,line);
  }
  getline(file,line);  // ENDOFSECTION
  cf_assert(line.compare("ENDOFSECTION"));
  
  
  // flush all buffers
  quad2D_buffer->flush();
  triag2D_buffer->flush();
}

//////////////////////////////////////////////////////////////////////////////

void NeuReader::read_groups(std::fstream& file)
{
  std::string line;
  int dummy;
  
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
  // Create Region for each group
  m_mesh->create_region("regions");
  boost::shared_ptr<CRegion> regions = m_mesh->get_component<CRegion>("regions");
  BOOST_FOREACH(GroupData& group, groups)
  {
    regions->create_region(group.ELMMAT);
    boost::shared_ptr<CRegion> region = regions->get_component<CRegion>(group.ELMMAT);
    
    // Create regions for each element type in each group-region
    std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer;
    BOOST_FOREACH(std::string& type, m_supported_types)
    {
      region->create_region_with_elementType(type);
      buffer[type]=boost::shared_ptr<CTable::Buffer>
        (new CTable::Buffer(region->get_component<CRegion>(type)->get_component<CTable>("table")->create_buffer()));
    }
    
    // Copy elements from tmp_region in the correct region
    BOOST_FOREACH(Uint global_element, group.ELEM)
    {
      boost::shared_ptr<CRegion> tmp_region = m_global_to_tmp[global_element].first;
      Uint local_element = m_global_to_tmp[global_element].second;
      buffer[tmp_region->name()]->add_row(tmp_region->get_component<CTable>("table")->get_table()[local_element]);
    }
    
  }
  
  // truely deallocate this vector
  std::vector<Region_TableIndex_pair>().swap (m_global_to_tmp);
}

//////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF
