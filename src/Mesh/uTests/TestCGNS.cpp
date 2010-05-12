#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "cgnslib.h"

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/OSystem.hpp"
#include "Common/LibLoader.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/MeshReader.hpp"
#include "Mesh/MeshWriter.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct TestCGNS_Fixture
{
  /// common setup for each test case
  TestCGNS_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestCGNS_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  
  /// These are handy functions that should maybe be implemented somewhere easily accessible.
  
  /// create a Real vector with 2 coordinates
  RealVector create_coord(const Real& x, const Real& y) {
    RealVector coordVec(2);
    coordVec[XX]=x;
    coordVec[YY]=y;
    return coordVec;
  }
  
  /// create a Uint vector with 4 node ID's
  std::vector<Uint> create_quad(const Uint& A, const Uint& B, const Uint& C, const Uint& D) {
    Uint quad[] = {A,B,C,D};
    std::vector<Uint> quadVec;
    quadVec.assign(quad,quad+4);
    return quadVec;
  }
  
  /// create a Uint vector with 3 node ID's
  std::vector<Uint> create_triag(const Uint& A, const Uint& B, const Uint& C) {
    Uint triag[] = {A,B,C};
    std::vector<Uint> triagVec;
    triagVec.assign(triag,triag+3);
    return triagVec;
  }
  
  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestCGNS_TestSuite, TestCGNS_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors )
{
  
  boost::shared_ptr<CMeshReader> meshreader ( new CMeshReader  ( "meshreader" ) );
  meshreader->set_reader("Mesh::CGNS::Reader");
 
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestCGNSLib )
{
 int maxelemi = 20*16*8;
 int maxelemj = 1216;

  double x[21*17*9],y[21*17*9],z[21*17*9];
  int isize[3][1],ielem[maxelemi][4],jelem[maxelemj][4];
  int ni,nj,nk,iset,i,j,k,index_file,icelldim,iphysdim;
  int index_base,index_zone,index_coord,ielem_no,nelem_start;
  int ifirstnode,nelem_end,nbdyelem,index_section;
  char basename[33],zonename[33];

/* create gridpoints for simple example: */
  ni=21;
  nj=17;
  iset=0;
    for (j=1; j <=nj; j++)
    {
      for (i=1; i <= ni; i++)
      {
        x[iset]=(float)i-1.;
        y[iset]=(float)j-1.;
        iset=iset+1;
      }
    }
  printf("\ncreated simple 2-D grid points\n");

/* WRITE X, Y, Z GRID POINTS TO CGNS FILE */
/* open CGNS file for write */
  cg_open("grid_c.cgns",CG_MODE_WRITE,&index_file);
/* create base (user can give any name) */
  strcpy(basename,"Base");
  icelldim=2;
  iphysdim=2;
  cg_base_write(index_file,basename,icelldim,iphysdim,&index_base);
/* define zone name (user can give any name) */
  strcpy(zonename,"Zone  1");
/* vertex size */
  isize[0][0]=ni*nj;
/* cell size */
  isize[1][0]=(ni-1)*(nj-1);
/* boundary vertex size (zero if elements not sorted) */
  isize[2][0]=0;
/* create zone */
  cg_zone_write(index_file,index_base,zonename,isize[0],Unstructured,&index_zone);
/* write grid coordinates (user must use SIDS-standard names here) */
  cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateX",x,&index_coord);
  cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateY",y,&index_coord);
  //cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateZ",z,&index_coord);
/* set element connectivity: */
/* ---------------------------------------------------------- */
/* do all the HEXA_8 elements (this part is mandatory): */
/* maintain SIDS-standard ordering */
  ielem_no=0;
/* index no of first element */
  nelem_start=1;

    for (j=1; j < nj; j++)
    {
      for (i=1; i < ni; i++)
      {
/*
in this example, due to the order in the node numbering, the
hexahedral elements can be reconstructed using the following
relationships:
*/
        ifirstnode=i+(j-1)*ni;
        ielem[ielem_no][0]=ifirstnode;
        ielem[ielem_no][1]=ifirstnode+1;
        ielem[ielem_no][2]=ifirstnode+1+ni;
        ielem[ielem_no][3]=ifirstnode+ni;
        ielem_no=ielem_no+1;
      }
    }
/* index no of last element (=2560) */
  nelem_end=ielem_no;
  if (nelem_end > maxelemi)
  {
    printf("\nError, must increase maxelemi to at least %d\n",nelem_end);
    exit(0);
  }
/* unsorted boundary elements */
  nbdyelem=0;
/* write HEXA_8 element connectivity (user can give any name) */
  cg_section_write(index_file,index_base,index_zone,"Elem",QUAD_4,nelem_start,  \
                   nelem_end,nbdyelem,ielem[0],&index_section);
/* ---------------------------------------------------------- */

/*
 do boundary (QUAD) elements (this part is optional,
 but you must do it if you eventually want to define BCs
 at element faces rather than at nodes):
 maintain SIDS-standard ordering
 */
 /* INFLOW: */
     ielem_no=0;
 /* index no of first element */
     nelem_start=nelem_end+1;
     i=1;
     for (k=1; k < nk; k++)
     {
       for (j=1; j < nj; j++)
       {
         ifirstnode=i+(j-1)*ni+(k-1)*ni*nj;
         jelem[ielem_no][0]=ifirstnode;
         jelem[ielem_no][1]=ifirstnode+ni*nj;
         jelem[ielem_no][2]=ifirstnode+ni*nj+ni;
         jelem[ielem_no][3]=ifirstnode+ni;
         ielem_no=ielem_no+1;
       }
     }
 /* index no of last element */
     nelem_end=nelem_start+ielem_no-1;
     if (ielem_no > maxelemj)
     {
       printf("\nError, must increase maxelemj to at least %d\n",ielem_no);
       exit(0);
     }
 /* write QUAD element connectivity for inflow face (user can give any name) */
     cg_section_write(index_file,index_base,index_zone,"InflowElem",QUAD_4,nelem_start,  \
                      nelem_end,nbdyelem,jelem[0],&index_section);
 /* OUTFLOW: */
     ielem_no=0;
 /* index no of first element */
     nelem_start=nelem_end+1;
     i=ni-1;
     for (k=1; k < nk; k++)
     {
       for (j=1; j < nj; j++)
       {
         ifirstnode=i+(j-1)*ni+(k-1)*ni*nj;
         jelem[ielem_no][0]=ifirstnode+1;
         jelem[ielem_no][1]=ifirstnode+1+ni;
         jelem[ielem_no][2]=ifirstnode+ni*nj+1+ni;
         jelem[ielem_no][3]=ifirstnode+ni*nj+1;
         ielem_no=ielem_no+1;
       }
     }
 /* index no of last element */
     nelem_end=nelem_start+ielem_no-1;
     if (ielem_no > maxelemj)
     {
       printf("\nError, must increase maxelemj to at least %d\n",ielem_no);
       exit(0);
     }
 /* write QUAD element connectivity for outflow face (user can give any name) */
     cg_section_write(index_file,index_base,index_zone,"OutflowElem",QUAD_4,nelem_start,  \
                      nelem_end,nbdyelem,jelem[0],&index_section);
 /* SIDEWALLS: */
     ielem_no=0;
 /* index no of first element */
     nelem_start=nelem_end+1;
     j=1;
     for (k=1; k < nk; k++)
     {
       for (i=1; i < ni; i++)
       {
         ifirstnode=i+(j-1)*ni+(k-1)*ni*nj;
         jelem[ielem_no][0]=ifirstnode;
         jelem[ielem_no][1]=ifirstnode+ni*nj;
         jelem[ielem_no][2]=ifirstnode+ni*nj+1;
         jelem[ielem_no][3]=ifirstnode+1;
         ielem_no=ielem_no+1;
       }
     }
     j=nj-1;
     for (k=1; k < nk; k++)
     {
       for (i=1; i < ni; i++)
       {
         ifirstnode=i+(j-1)*ni+(k-1)*ni*nj;
         jelem[ielem_no][0]=ifirstnode+1+ni;
         jelem[ielem_no][1]=ifirstnode+ni;
         jelem[ielem_no][2]=ifirstnode+ni*nj+ni;
         jelem[ielem_no][3]=ifirstnode+ni*nj+1+ni;
         ielem_no=ielem_no+1;
       }
     }
     k=1;
     for (j=1; j < nj; j++)
     {
       for (i=1; i < ni; i++)
       {
         ifirstnode=i+(j-1)*ni+(k-1)*ni*nj;
         jelem[ielem_no][0]=ifirstnode;
         jelem[ielem_no][1]=ifirstnode+1;
         jelem[ielem_no][2]=ifirstnode+1+ni;
         jelem[ielem_no][3]=ifirstnode+ni;
         ielem_no=ielem_no+1;
       }
     }
     k=nk-1;
     for (j=1; j < nj; j++)
     {
       for (i=1; i < ni; i++)
       {
         ifirstnode=i+(j-1)*ni+(k-1)*ni*nj;
         jelem[ielem_no][0]=ifirstnode+ni*nj;
         jelem[ielem_no][1]=ifirstnode+ni*nj+ni;
         jelem[ielem_no][2]=ifirstnode+ni*nj+1+ni;
         jelem[ielem_no][3]=ifirstnode+ni*nj+1;
         ielem_no=ielem_no+1;
       }
     }
 /* index no of last element */
     nelem_end=nelem_start+ielem_no-1;
     if (ielem_no > maxelemj)
     {
       printf("\nError, must increase maxelemj to at least %d\n",ielem_no);
       exit(0);
     }
 /* write QUAD element connectivity for sidewall face (user can give any name) */
     cg_section_write(index_file,index_base,index_zone,"SidewallElem",QUAD_4,nelem_start,  \
                      nelem_end,nbdyelem,jelem[0],&index_section);
 /* ----------------------------------------------------------
/* close CGNS file */
  cg_close(index_file);
  printf("\nSuccessfully wrote unstructured grid to file grid_c.cgns\n");
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

