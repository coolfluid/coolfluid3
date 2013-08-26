
#include <iostream>
#include <sstream>
#include <algorithm>
#include "smurf.h"

using namespace std;
namespace SmURF {

// ------------------------------------------------------------------------ //
// utilities

template< typename T >
void bwrite(FILE *F, T v, bool reverse)
{
  if (reverse) {
    const unsigned len = sizeof(v);
    char *p = (char*) &v;
    char r_buffer[256];  // should be enough...
    for (unsigned i=0; i<len; ++i)
      r_buffer[i] = p[len-1-i];
    for (unsigned i=0; i<len; ++i)
      p[i] = r_buffer[i];
  }
  size_t s = fwrite(&v,sizeof(v),1,F);
  s=s;
}


template< >
void bwrite< string >(FILE *F, string v, bool reverse)
{
  for (unsigned i=0; i<v.size(); ++i)
    bwrite< int >(F,v[i],reverse);
  bwrite< int >(F,0,reverse);
}


template< typename T >
T bread(ifstream& F)
{
  T v;
  F.read((char*)(&v),sizeof(T));
  return v;
}


template< >
string bread(ifstream& F)
{
  string s;
  while (true) {
    const char c = bread< int >(F);
    if (!c)
      break;
    s.push_back(c);
  }
  return s;
}


// ------------------------------------------------------------------------ //
// MeshWriter

MeshWriter::MeshWriter(const string& fname, const DataType _datatype, const bool _reverse, const unsigned _version, const double _solutiontime) :
  m_file(NULL),
  m_datatype(_datatype),
  m_reverse(_reverse),
  m_version(_version),
  m_solutiontime(_solutiontime)
{
  // open file for writing
  m_file = fopen(fname.c_str(),"wb");
  if (m_file==NULL) {
    throw "SmURF: error accessing file!";
  }
}


MeshWriter::~MeshWriter()
{
  // close file
  fclose(m_file);
  m_file = NULL;
}


void MeshWriter::writeMainHeader(const string& htitle, const vector< string >& vnames)
{
  m_nvars = (unsigned) vnames.size();

  // main header information
  ostringstream s;
  s << "#!TDV" << m_version;
  fprintf(m_file,"%s",s.str().c_str());

  bwrite< int >(m_file,1,m_reverse);          // byte order of reader
  if (m_version>107)
    bwrite< int >(m_file,0,m_reverse);        // filetype: FULL
  bwrite< string >(m_file,htitle,m_reverse);  // title
  bwrite< int >(m_file,m_nvars,m_reverse);    // variables number
  for (unsigned i=0; i<m_nvars; ++i)          // variables names
    bwrite(m_file,vnames[i],m_reverse);

  m_eohmarker = false;  // only set EOH marker when starting data section
}


void MeshWriter::writeZoneHeader(const ZoneType& type,
                                 const ZonePack& pack,
                                 const std::string& title,
                                 const unsigned I, const unsigned J, const unsigned K,
                                 const int& strand_id)
{
  bwrite< float >(m_file,ZONEMARKER,m_reverse);

  // zone header information
  bwrite< string >(m_file,title,m_reverse);         // title
  bwrite< int >   (m_file,-1,m_reverse);            // BAD_SET_VALUE
  bwrite< int >   (m_file,strand_id,m_reverse);     // strand ID: pending assignment by Tecplot
  bwrite< double >(m_file,m_solutiontime,m_reverse);// solution time
  bwrite< int >   (m_file,-1,m_reverse);            // color
  bwrite< int >   (m_file,type,m_reverse);          // type
  if (m_version<112)
    bwrite< int > (m_file,pack,m_reverse);          // data packing
  bwrite< int >   (m_file,0,m_reverse);             // specify var location: no cell centered vars
  if (m_version>107)
    bwrite< int > (m_file,0,m_reverse);             // no face neighbor array
  bwrite< int >   (m_file,0,m_reverse);             // no face neighbor connections

  if (type==ORDERED) {
    bwrite< int >(m_file,max< unsigned >(I,1),m_reverse);
    bwrite< int >(m_file,max< unsigned >(J,1),m_reverse);
    bwrite< int >(m_file,max< unsigned >(K,1),m_reverse);
  }
  else {
    if (type==FEPOLYGON || type==FEPOLYHEDRON) {
      cerr << "SmURF: error FEPOLYGON and FEPOLYHEDRON are not implemented!" << endl;
      throw 42;
      const unsigned L = 9,
                     M = 9;
      bwrite< int >(m_file,L,m_reverse);                       // NumFaces
      bwrite< int >(m_file,type==FEPOLYGON? L*2:M,m_reverse);  // Number of face nodes
      bwrite< int >(m_file,0,m_reverse);  // Number of boundary faces
      bwrite< int >(m_file,0,m_reverse);  // Number of boundary connections
    }
    else {
      bwrite< int >(m_file,I,m_reverse);
      bwrite< int >(m_file,J,m_reverse);
      bwrite< int >(m_file,0,m_reverse);  // no ICellDim, reserved for the future
      bwrite< int >(m_file,0,m_reverse);  // ... JCellDim
      bwrite< int >(m_file,0,m_reverse);  // ... KCellDim
    }
  }

  bwrite< int >(m_file,0,m_reverse);    // no auxiliary data
}


void MeshWriter::writeZoneData(const ZoneType& type, const ZonePack& pack, const vector< vector< unsigned > >& ve, const vector< vector< double > >& vv, const int sharefrom)
{
  // place end of header marker before writing zone data sections
  if (!m_eohmarker) {
    bwrite< float >(m_file,EOHMARKER,m_reverse);
    m_eohmarker = true;
  }
  bwrite< float >(m_file,ZONEMARKER,m_reverse);

  // variable data types
  for (unsigned N=0; N<m_nvars; ++N)
    bwrite< int >(m_file,m_datatype,m_reverse);
  bwrite< int >(m_file,0,m_reverse);  // no passive variables

  // shared variables
  const bool isshared = (sharefrom>=0);
  bwrite< int >(m_file,(isshared? 1:0),m_reverse);
  if (isshared)
    for (unsigned N=0; N<m_nvars; ++N)
      bwrite< int >(m_file,sharefrom,m_reverse);
  bwrite< int >(m_file,-1,m_reverse);  // no shared connectivity

  // data section
  if (!isshared) {
    const unsigned Nnode = vv[0].size();

    // variables min/max values
    for (unsigned N=0; N<m_nvars; ++N) {
      bwrite< double >(m_file,*min_element(vv[N].begin(),vv[N].end()),m_reverse);
      bwrite< double >(m_file,*max_element(vv[N].begin(),vv[N].end()),m_reverse);
    }

    if (pack==POINT && m_version<112) {

      for (unsigned i=0; i<Nnode; ++i)
        for (unsigned N=0; N<m_nvars; ++N)
          if (m_datatype==DOUBLE)  bwrite< double >(m_file,vv[N][i],m_reverse);
          else                     bwrite< float >(m_file,(float) vv[N][i],m_reverse);

    }
    else {

      for (unsigned N=0; N<m_nvars; ++N)
        for (unsigned i=0; i<Nnode; ++i)
          if (m_datatype==DOUBLE)  bwrite< double >(m_file,vv[N][i],m_reverse);
          else                     bwrite< float >(m_file,(float) vv[N][i],m_reverse);

    }
  }

  // connectivity
  if (type!=ORDERED) {
    if (type==FEPOLYGON || type==FEPOLYHEDRON) {

      // face map data
      cerr << "SmURF: error FEPOLYGON and FEPOLYHEDRON are not implemented!" << endl;
      throw 42;
      bwrite< int >(m_file,9,m_reverse);  // INT32*F   | Face node offsets into the face nodes array below. Does not exist for FEPOLYGON zones.  F = NumFaces+1.
      bwrite< int >(m_file,9,m_reverse);  // INT32*FN  | Face nodes array containing the node numbers for all nodes in all faces.  FN = total number of face nodes.
      bwrite< int >(m_file,9,m_reverse);  // INT32*F   | Elements on the left side of all faces.  Boundary faces use a negative value which is the negated offset into the face boundary connection offsets array.  A value of "-1" indicates there is no left element.  F = NumFaces.
      bwrite< int >(m_file,9,m_reverse);  // INT32*F   | Elements on the right side of all faces.  F = NumFaces.

    }
    else {

      // zone connectivity data
      const unsigned Nelem = ve.size();
      const unsigned L = ve[0].size();
      for (unsigned j=0; j<Nelem; ++j)
        for (unsigned i=0; i<L; ++i)
          bwrite< unsigned >(m_file,ve[j][i],m_reverse);

    }
  }
}

// ------------------------------------------------------------------------ //
// MeshReader

MeshReader::MeshReader(const string& fname)
{
  m_file.open(fname.c_str(),ios::binary);
  if (!m_file) {
    throw "SmURF: error accessing file!";
  }
}


MeshReader::~MeshReader()
{
  // close file
  m_file.close();
}


void MeshReader::readMainHeader(string& htitle, vector< string >& hvnames)
{
  // version
  string s(8,'?');
  for (unsigned i=0; i<8; ++i)
    m_file >> s[i];
  istringstream ss(s.substr(5));
  ss >> m_version;
  //std::cout << "SmURF: version number: \"" << m_version << "\"" << std::endl;

  // byte order, filetype and title
  int i;
  i = bread< int >(m_file);    // 1
  if (m_version>107)
    i = bread< int >(m_file);  // 0
  htitle = bread< string >(m_file);

  // variables
  m_nvars = (unsigned) bread< int >(m_file);
  hvnames.resize(m_nvars);
  for (unsigned i=0; i<hvnames.size(); ++i)
    hvnames[i] = bread< string >(m_file);
}


vector< TecZone > MeshReader::readZoneHeaders()
{
  vector< TecZone > vzones;
  float marker;
  for (marker=bread< float >(m_file); marker==ZONEMARKER; marker=bread< float >(m_file)) {
    TecZone z = {"",0.,AUTO,ORDERED,BLOCK,0,0,0};

    int i;
    z.title = bread< string >(m_file);      // title
    i = bread< int >(m_file);               // BAD_SET_VALUE
    i = bread< int >(m_file);               // strandid: static
    z.time  = bread< double >(m_file);      // solution time
    z.color = bread< ZoneColor >(m_file);   // color
    z.type  = bread< ZoneType >(m_file);    // type
    if (m_version<112)
      z.pack = bread< ZonePack >(m_file);   // data packing
    i = bread< int >(m_file);               // no cell centered vars
    i = bread< int >(m_file);               // no auto-generated face neighbor array
    if (m_version>107)  // i'm not sure this is correct
      i = bread< int >(m_file);             // no FaceNeighborConnections

    if (z.type==ORDERED) {
      z.i = (unsigned) bread< int >(m_file);
      z.j = (unsigned) bread< int >(m_file);
      z.k = (unsigned) bread< int >(m_file);
    }
    else {
      z.i = (unsigned) bread< int >(m_file);
      z.j = (unsigned) bread< int >(m_file);
      i = bread< int >(m_file);  // no ICellDim, reserved for the future
      i = bread< int >(m_file);  // ... JCellDim
      i = bread< int >(m_file);  // ... KCellDim
    }

    while (bread< int >(m_file)) {  // skip auxiliary data
      bread< string >(m_file);      // ... aux name
      bread< int >(m_file);         // ... aux value format
      bread< string >(m_file);      // ... aux value string
    }

    vzones.push_back(z);
  }

  if (marker!=EOHMARKER) { std::cout << "SmURF: did not detect EOHMARKER!" << std::endl; exit(666); }
  return vzones;
}


void MeshReader::readZoneData(const TecZone& z, vector< vector< unsigned > >& ve, vector< vector< double > >& vv)
{
  double d;
  int i;

  const float marker = bread< float >(m_file);
  if (marker!=ZONEMARKER) {
    cerr << "SmURF: corrupt file!" << endl;
    throw 42;
  }

  vector< DataType > vtypes(m_nvars,FLOAT);
  for (unsigned j=0; j<m_nvars; ++j)
    vtypes[j] = bread< DataType >(m_file);  // variable data types
  i = bread< int >(m_file);                 // no passive variables

  // shared variables
  const bool isshared = (bread< int >(m_file)!=0);
  int sharefrom;
  if (isshared)
    for (unsigned N=0; N<m_nvars; ++N)
      sharefrom = bread< int >(m_file);  // shares only from first zone only (yet), sharefrom=0
  i = bread< int >(m_file);  // no shared connectivity

  // data section
  if (!(isshared)) {
    const unsigned Nnode = z.i;
    vv.assign(m_nvars,vector< double >(Nnode,0.));

    // variables min/max values
    for (unsigned N=0; N<m_nvars; ++N) {
      d = bread< double >(m_file);
      d = bread< double >(m_file);
    }

    if (z.pack==BLOCK) {

      for (unsigned N=0; N<m_nvars; ++N)
        for (unsigned i=0; i<Nnode; ++i)
          if (vtypes[N]==DOUBLE)  vv[N][i] = bread< double >(m_file);
          else                    vv[N][i] = (double) bread< float >(m_file);

    }
    else if (z.pack==POINT) {

      for (unsigned i=0; i<Nnode; ++i)
        for (unsigned N=0; N<m_nvars; ++N)
          if (vtypes[N]==DOUBLE)  vv[N][i] = bread< double >(m_file);
          else                    vv[N][i] = (double) bread< float >(m_file);

    }
  }

  // FE connectivity
  if (z.type!=ORDERED) {
    const unsigned Nelem = z.j;
    const unsigned L = (z.type==FELINESEG?       2:
                       (z.type==FETRIANGLE?      3:
                       (z.type==FEQUADRILATERAL? 4:
                       (z.type==FETETRAHEDRON?   4:
                       (z.type==FEBRICK?         8:0 )))));
    ve.assign(Nelem,vector< unsigned >(L,0));
    for (unsigned j=0; j<Nelem; ++j)
      for (unsigned i=0; i<L; ++i)
        ve[j][i] = bread< unsigned >(m_file);
  }
}

// ------------------------------------------------------------------------ //

}

