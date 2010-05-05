#include "Common/ObjectProvider.hpp"
#include "Mesh/Neu/NeuReader.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"

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
}

//////////////////////////////////////////////////////////////////////////////

void NeuReader::read_headerData(std::fstream& file)
{
  CFAUTOTRACE;
    
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
  // skip next two lines
  for (unsigned i=0; i<2; ++i)
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
}

//////////////////////////////////////////////////////////////////////////////

void NeuReader::read_connectivity(std::fstream& file)
{
  // make temporary regions for each element type possible
  m_mesh->create_region("tmp_region_collection");
  boost::shared_ptr<CRegion> tmp = m_mesh->get_component<CRegion>("tmp_region_collection");
  boost::shared_ptr<CTable::Buffer> buffer;

  // quadrilateral
  tmp->create_region_with_elementType("Quad2D");
  boost::shared_ptr<CTable::Buffer> quad2D_buffer 
      (new CTable::Buffer(tmp->get_component<CRegion>("Quad2D")->get_component<CTable>("table")->create_buffer()));

  // triangle
  tmp->create_region_with_elementType("Triag2D");
  boost::shared_ptr<CTable::Buffer> triag2D_buffer 
      (new CTable::Buffer(tmp->get_component<CRegion>("Triag2D")->get_component<CTable>("table")->create_buffer()));
  
  
  // skip next two lines
  std::string line;
  for (unsigned i=0; i<2; ++i)
    getline(file,line);
    
  // read every line and store the connectivity in the correct region through the buffer
  for (Uint i=0; i<m_headerData.NUMNP; ++i) {
    // element description
    Uint elementNumber, elementType, nbElementNodes;
    file >> elementNumber >> elementType >> nbElementNodes;
    elementNumber--;
    if      (elementType==2 && nbElementNodes==4) buffer = quad2D_buffer; // quadrilateral
    else if (elementType==3 && nbElementNodes==3) buffer = triag2D_buffer;// triangle
    else if (elementType==4 && nbElementNodes==8) ;// brick
    else if (elementType==5 && nbElementNodes==6) ;// wedge (prism)
    else if (elementType==6 && nbElementNodes==4) ;// tetrahedron
    else if (elementType==7 && nbElementNodes==5) ;// pyramid
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
    // finish the line
    getline(file,line);
  }
  
  // flush all buffers
  quad2D_buffer->flush();
  triag2D_buffer->flush();
}

//////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF
