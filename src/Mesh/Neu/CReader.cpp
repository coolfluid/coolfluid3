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
: CMeshReader(name)
{
  BUILD_COMPONENT;

  m_supported_types.reserve(2);
  m_supported_types.push_back("P1-Line1D");
  m_supported_types.push_back("P1-Line2D");
  m_supported_types.push_back("P1-Line3D");
  m_supported_types.push_back("P1-Quad2D");
  m_supported_types.push_back("P1-Quad3D");
  m_supported_types.push_back("P1-Triag2D");
  m_supported_types.push_back("P1-Triag3D");
  m_supported_types.push_back("P1-Hexa3D");
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
    // else if (elementType==5 && nbElementNodes==6) ;// wedge (prism)
    // else if (elementType==6 && nbElementNodes==4) ;// tetrahedron
    // else if (elementType==7 && nbElementNodes==5) ;// pyramid
    else {
      CFerr << "error: no support for element type/nodes " << elementType << "/" << nbElementNodes << CFflush;
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
    
    // Remove tmp region from component
    Component::Ptr tmp = m_mesh->remove_component("tmp");
    
    /// @todo check if the following really deletes it from memory
    tmp.reset();
    
  }

  // truely deallocate this vector
  std::vector<Region_TableIndex_pair>().swap (m_global_to_tmp);
  
  // Remove regions with empty connectivity tables
  remove_empty_leaf_regions(regions);
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_boundaries(std::fstream& file)
{
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
      CFerr << "error: supports only boundary condition data 1 (element/cell): page C-11 of user's guide" << CFendl;
      throw 42;
    }
    if (IBCODE1!=6) {
      CFerr << "error: supports only IBCODE1 6 (ELEMENT_SIDE)" << CFendl;
      throw 42;
    }

    // boundary connectivity here
    //vector< GElement > e2n(NENTRY);

    // read boundary elements connectivity
    // vector< unsigned > nbelems(PYRAMID4+1,0); // nb. elements per type
    for (int i=0; i<NENTRY; ++i) {
      int ELEM, ETYPE, FACE;
      file >> ELEM >> ETYPE >> FACE;
      /*
      // element nodes and face nodes/type
      const vector< unsigned >& en = vgelems[ ELEM-1 ].n;
      vector< unsigned >& fn = e2n[i].n;
      mtype&              ft = e2n[i].t;
      if      (ETYPE==2 && en.size()==4)  ft = FELINESEG;        // quadrilateral faces
      else if (ETYPE==3 && en.size()==3)  ft = FELINESEG;        // triangle faces
      else if (ETYPE==4 && en.size()==8)  ft = FEQUADRILATERAL;  // brick faces
      else if (ETYPE==5 && en.size()==6)  ft = (FACE<4? FEQUADRILATERAL : FETRIANGLE);  // wedge (prism) faces
      else if (ETYPE==6 && en.size()==4)  ft = FETRIANGLE;       // tetrahedron faces
      else if (ETYPE==7 && en.size()==5)  ft = (FACE<2? FEQUADRILATERAL : FETRIANGLE);  // pyramid faces
      else {
        cerr << "error: reference for an unexpected volume element" << endl;
        throw 42;
      }
      ++nbelems[(unsigned) ft];  // add a face element of this type
      fn.assign(ft==FELINESEG?       2:
               (ft==FEQUADRILATERAL? 4:
               (ft==FETRIANGLE?      3:
                                     1 )), 0);

      if (ETYPE==2) { // quadrilateral faces
        switch (FACE) {
          case 1: fn[0] = en[0]; fn[1] = en[1]; break;
          case 2: fn[0] = en[1]; fn[1] = en[2]; break;
          case 3: fn[0] = en[2]; fn[1] = en[3]; break;
          case 4: fn[0] = en[3]; fn[1] = en[0]; break;
        }
      }
      else if (ETYPE==3) { // triangle faces
        switch (FACE) {
          case 1: fn[0] = en[0]; fn[1] = en[1]; break;
          case 2: fn[0] = en[1]; fn[1] = en[2]; break;
          case 3: fn[0] = en[2]; fn[1] = en[0]; break;
        }
      }
      else if (ETYPE==4) { // brick faces
        switch (FACE) {
          case 1: fn[0] = en[0]; fn[1] = en[1]; fn[2] = en[5]; fn[3] = en[4]; break;
          case 2: fn[0] = en[1]; fn[1] = en[3]; fn[2] = en[7]; fn[3] = en[5]; break;
          case 3: fn[0] = en[3]; fn[1] = en[2]; fn[2] = en[6]; fn[3] = en[7]; break;
          case 4: fn[0] = en[2]; fn[1] = en[0]; fn[2] = en[4]; fn[3] = en[6]; break;
          case 5: fn[0] = en[1]; fn[1] = en[0]; fn[2] = en[2]; fn[3] = en[3]; break;
          case 6: fn[0] = en[4]; fn[1] = en[5]; fn[2] = en[7]; fn[3] = en[6]; break;
        }
      }
      else if (ETYPE==5) { // wedge (prism) faces
        switch (FACE) {
          case 1: fn[0] = en[0]; fn[1] = en[1]; fn[2] = en[4]; fn[3] = en[3]; break;
          case 2: fn[0] = en[1]; fn[1] = en[2]; fn[2] = en[5]; fn[3] = en[4]; break;
          case 3: fn[0] = en[2]; fn[1] = en[0]; fn[2] = en[3]; fn[3] = en[5]; break;
          case 4: fn[0] = en[0]; fn[1] = en[2]; fn[2] = en[1]; break;
          case 5: fn[0] = en[3]; fn[1] = en[4]; fn[2] = en[5]; break;
        }
      }
      else if (ETYPE==6) { // tetrahedron faces
        switch (FACE) {
          case 1: fn[0] = en[1]; fn[1] = en[0]; fn[2] = en[2]; break;
          case 2: fn[0] = en[0]; fn[1] = en[1]; fn[2] = en[3]; break;
          case 3: fn[0] = en[1]; fn[1] = en[2]; fn[2] = en[3]; break;
          case 4: fn[0] = en[2]; fn[1] = en[0]; fn[2] = en[3]; break;
        }
      }
      else if (ETYPE==7) { // pyramid faces
        switch (FACE) {
          case 1: fn[0] = en[0]; fn[1] = en[2]; fn[2] = en[3]; fn[3] = en[1]; break;
          case 2: fn[0] = en[0]; fn[1] = en[1]; fn[2] = en[4]; break;
          case 3: fn[0] = en[1]; fn[1] = en[3]; fn[2] = en[4]; break;
          case 4: fn[0] = en[3]; fn[1] = en[2]; fn[2] = en[4]; break;
          case 5: fn[0] = en[2]; fn[1] = en[0]; fn[2] = en[4]; break;
        }
      }
      */
      getline(file,line);  // finish the line (read new line)
    }
    getline(file,line);  // ENDOFSECTION

/*
    // add a new zone, splitting according to type (if necessary)
    // count element types
    vector< mtype    > felemstypes;
    vector< unsigned > felemsnb;
    for (unsigned t=0; t<nbelems.size(); ++t)
      if (nbelems[t]>0) {
        felemstypes.push_back((mtype) t);
        felemsnb.push_back(nbelems[t]);
      }

    // set new zones, distinguishing different element types
    for (unsigned i=0; i<felemstypes.size(); ++i) {
      m.vz.push_back(mzone());
      mzone& z = m.vz.back();

      // set name, dimensionality, type and connectivity
      ostringstream name;
      name << NAME;
      if (felemstypes.size()>1)
        name << "_t" << felemstypes[i];
      z.n = name.str();
      z.t = felemstypes[i];

      // set new zone element-node connectivity
      z.e2n.resize(felemsnb[i]);
      for (unsigned j=0, k=0; j<e2n.size(); ++j)
        if (e2n[j].t==felemstypes[i])
          z.e2n[k++].n = e2n[j].n;
    }
*/

  }

}

//////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF
