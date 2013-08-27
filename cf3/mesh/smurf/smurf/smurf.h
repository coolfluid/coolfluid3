#ifndef smurf_h
#define smurf_h

#include <fstream>
#include <vector>

// based on PREPLOT V11.2-0  06/05/2007
// Copyright (C) Tecplot, Inc. 1987-2008


namespace SmURF {


enum ZoneColor { AUTO=-1, BLACK=0, RED, GREEN, BLUE, CYAN, YELLOW, PURPLE, WHITE };
enum ZoneType  { ORDERED=0, FELINESEG=1, FETRIANGLE=2, FEQUADRILATERAL=3, FETETRAHEDRON=4, FEBRICK=5, FEPOLYGON=6, FEPOLYHEDRON=7 };
enum ZonePack  { BLOCK, POINT };
enum DataType  { RESERVED, FLOAT, DOUBLE };

struct TecZone {
  std::string title;
  double      time;
  ZoneColor   color;
  ZoneType    type;
  ZonePack    pack;
  unsigned    i, j, k;
};


class MeshWriter
{
 public:
  MeshWriter(const std::string& fname, const DataType _datatype=DOUBLE, const bool _reverse=false, const unsigned _version=107, const double _solutiontime=0.);
  ~MeshWriter();
  void writeMainHeader(const std::string& htitle, const std::vector< std::string >& hvnames);
  void writeZoneHeader(const ZoneType& type, const ZonePack& pack, const std::string& title, const unsigned I=1, const unsigned J=1, const unsigned K=1, const int &strand_id=int(-2));
  void writeZoneData  (const ZoneType& type, const ZonePack& pack, const std::vector< std::vector< unsigned > >& ve, const std::vector< std::vector< double > >& vv=std::vector< std::vector< double > >(), const int sharefrom=0);
 private:
  FILE*          m_file;          // file pointer
  const DataType m_datatype;      // data type (for all variables)
  const bool     m_reverse;       // byte reversal
  const unsigned m_version;       // file version
  const double   m_solutiontime;  // solution time
  unsigned       m_nvars;         // number of variables
  bool           m_eohmarker;     // end-of-header marker
};


class MeshReader
{
 public:
  MeshReader(const std::string& fname);
  ~MeshReader();
  void readMainHeader(std::string& htitle, std::vector< std::string >& hvnames);
  std::vector< TecZone > readZoneHeaders();
  void readZoneData(const TecZone& z, std::vector< std::vector< unsigned > >& ve, std::vector< std::vector< double > >& vv);
 private:
  std::ifstream m_file;  // file stream
  unsigned  m_version;   // file version
  unsigned  m_nvars;     // number of variables
};


}

#endif

