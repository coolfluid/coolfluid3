// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include <iostream>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CGNS"
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "rapidxml/rapidxml.hpp"


#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/OSystem.hpp"
#include "common/LibLoader.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "common/Table.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Connectivity.hpp"

#include "mesh/CGNS/Shared.hpp"


using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::CGNS;

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

  std::string xml_config;
  rapidxml::xml_document<char> doc;
  char* ctext;

  rapidxml::xml_node<char>* parsed_config()
  {
    ctext = doc.allocate_string(xml_config.c_str());
    doc.parse< rapidxml::parse_no_data_nodes >(ctext);
    return doc.first_node();
  }
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestCGNS_TestSuite, TestCGNS_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors )
{
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.CGNS.Reader","meshreader");
  BOOST_CHECK_EQUAL(meshreader->name(),"meshreader");
  BOOST_CHECK_EQUAL(meshreader->get_format(),"CGNS");
}

////////////////////////////////////////////////////////////////////////////////

#define NI 17
#define NJ 9
#define NK 5

BOOST_AUTO_TEST_CASE( TestLibCGNS )
{

 int maxelemi = (NI-1)*(NJ-1)*(NK-1);
 double x[NI*NJ*NK],y[NI*NJ*NK],z[NI*NJ*NK];
 cgsize_t isize[3][1],ielem[maxelemi][8];
 int ni,nj,nk,iset,i,j,k,index_file,icelldim,iphysdim;
 int index_base,index_zone,index_coord,ielem_no,nelem_start;
 int ifirstnode,nelem_end,nbdyelem,index_section;
 char basename[33],zonename[33];

/* create gridpoints for simple example: */
 ni=NI;
 nj=NJ;
 nk=NK;
 iset=0;
 for (k=1; k <= nk; k++)
 {
   for (j=1; j <=nj; j++)
   {
     for (i=1; i <= ni; i++)
     {
       x[iset]=(float)i-1.;
       y[iset]=(float)j-1.;
       z[iset]=(float)k-1.;
       iset=iset+1;
     }
   }
 }

/* WRITE X, Y, Z GRID POINTS TO CGNS FILE */
/* open CGNS file for write */
 CALL_CGNS(cg_open("grid_c.cgns",CG_MODE_WRITE,&index_file));
/* create base (user can give any name) */
 strcpy(basename,"Base");
 icelldim=3;
 iphysdim=3;
 CALL_CGNS(cg_base_write(index_file,basename,icelldim,iphysdim,&index_base));
/* define zone name (user can give any name) */
 strcpy(zonename,"Zone 1");
/* vertex size */
 isize[0][0]=ni*nj*nk;
/* cell size */
 isize[1][0]=(ni-1)*(nj-1)*(nk-1);
/* boundary vertex size (zero if elements not sorted) */
 isize[2][0]=0;
/* create zone */
 CALL_CGNS(cg_zone_write(index_file,index_base,zonename,isize[0],CGNS_ENUMV( Unstructured ),&index_zone));
/* write grid coordinates (user must use SIDS-standard names here) */
 CALL_CGNS(cg_coord_write(index_file,index_base,index_zone,CGNS_ENUMV( RealDouble ),"CoordinateX",x,&index_coord));
 CALL_CGNS(cg_coord_write(index_file,index_base,index_zone,CGNS_ENUMV( RealDouble ),"CoordinateY",y,&index_coord));
 CALL_CGNS(cg_coord_write(index_file,index_base,index_zone,CGNS_ENUMV( RealDouble ),"CoordinateZ",z,&index_coord));
/* set element connectivity: */
/* ---------------------------------------------------------- */
/* do all the CGNS_ENUMV( HEXA_8 ) elements (this part is mandatory): */
/* maintain SIDS-standard ordering */
 ielem_no=0;
/* index no of first element */
 nelem_start=1;
 for (k=1; k < nk; k++)
 {
   for (j=1; j < nj; j++)
   {
     for (i=1; i < ni; i++)
     {
/*
in this example, due to the order in the node numbering, the
hexahedral elements can be reconstructed using the following
relationships:
*/
       ifirstnode=i+(j-1)*ni+(k-1)*ni*nj;
       ielem[ielem_no][0]=ifirstnode;
       ielem[ielem_no][1]=ifirstnode+1;
       ielem[ielem_no][2]=ifirstnode+1+ni;
       ielem[ielem_no][3]=ifirstnode+ni;
       ielem[ielem_no][4]=ifirstnode+ni*nj;
       ielem[ielem_no][5]=ifirstnode+ni*nj+1;
       ielem[ielem_no][6]=ifirstnode+ni*nj+1+ni;
       ielem[ielem_no][7]=ifirstnode+ni*nj+ni;
       ielem_no=ielem_no+1;
     }
   }
 }
/* index no of last element (=2560) */
 nelem_end=ielem_no;
 if (nelem_end > maxelemi)
 {
   printf("\nError, must increase maxelemi to at least %d\n",nelem_end);
   exit(0);
 }
 std::cout << "volume: " << nelem_start << " , " << nelem_end << std::endl;
/* unsorted boundary elements */
 nbdyelem=0;
/* write CGNS_ENUMV( HEXA_8 ) element connectivity (user can give any name) */
 CALL_CGNS(cg_section_write(index_file,index_base,index_zone,"Elem",CGNS_ENUMV( HEXA_8 ),nelem_start,  \
                  nelem_end,nbdyelem,ielem[0],&index_section));
/* ---------------------------------------------------------- */
int maxelemj = (NI)*(NJ)*(NK);
std::cout << "maxelemj = " << maxelemj << std::endl;
cgsize_t jelem[maxelemj][4];
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
 int inflow_start = nelem_start;
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
 int inflow_last = nelem_end;
 if (ielem_no > maxelemj)
 {
   printf("\nError, must increase maxelemj to at least %d\n",ielem_no);
   exit(0);
 }
/* write QUAD element connectivity for inflow face (user can give any name) */
 CALL_CGNS(cg_section_write(index_file,index_base,index_zone,"InflowElem",CGNS_ENUMV( QUAD_4 ),nelem_start,  \
                  nelem_end,nbdyelem,jelem[0],&index_section));
/* OUTFLOW: */
 ielem_no=0;
/* index no of first element */
 nelem_start=nelem_end+1;
 int outflow_start = nelem_start;
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
 int outflow_last = nelem_end;
 if (ielem_no > maxelemj)
 {
   printf("\nError, must increase maxelemj to at least %d\n",ielem_no);
   exit(0);
 }
/* write QUAD element connectivity for outflow face (user can give any name) */
 CALL_CGNS(cg_section_write(index_file,index_base,index_zone,"OutflowElem",CGNS_ENUMV( QUAD_4 ),nelem_start,  \
                  nelem_end,nbdyelem,jelem[0],&index_section));
/* SIDEWALLS: */
 ielem_no=0;
/* index no of first element */
 nelem_start=nelem_end+1; 
 int side_wall_start = nelem_start;
 j=1;
 for (k=1; k < nk; k++)
 {
   for (i=1; i < ni; i++)
   {
     ifirstnode=i+(j-1)*ni+(k-1)*ni*nj;
     jelem[ielem_no][0]=ifirstnode+1;
     jelem[ielem_no][1]=ifirstnode+ni*nj+1;
     jelem[ielem_no][2]=ifirstnode+ni*nj;
     jelem[ielem_no][3]=ifirstnode;
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
     jelem[ielem_no][0]=ifirstnode+ni;
     jelem[ielem_no][1]=ifirstnode+1+ni;
     jelem[ielem_no][2]=ifirstnode+1;
     jelem[ielem_no][3]=ifirstnode;
     ielem_no=ielem_no+1;
   }
 }
 k=nk-1;
 for (j=1; j < nj; j++)
 {
   for (i=1; i < ni; i++)
   {
     ifirstnode=i+(j-1)*ni+(k-1)*ni*nj;
     jelem[ielem_no][0]=ifirstnode+ni*nj+1;
     jelem[ielem_no][1]=ifirstnode+ni*nj+1+ni;    
     jelem[ielem_no][2]=ifirstnode+ni*nj+ni;
     jelem[ielem_no][3]=ifirstnode+ni*nj;
     ielem_no=ielem_no+1;
   }
 }
/* index no of last element */
 nelem_end=nelem_start+ielem_no-1;
 int side_wall_last = nelem_end;
 if (ielem_no > maxelemj)
 {
   printf("\nError, must increase maxelemj to at least %d\n",ielem_no);
   exit(0);
 }
/* write QUAD element connectivity for sidewall face (user can give any name) */
 CALL_CGNS(cg_section_write(index_file,index_base,index_zone,"SidewallElem",CGNS_ENUMV( QUAD_4 ),nelem_start,  \
                  nelem_end,nbdyelem,jelem[0],&index_section));
/* ---------------------------------------------------------- */

 // part 2: add boundary conditions
 int icount, n, index_bc;
 int maxcount(960);
 cgsize_t ipnts[maxcount];
 // BC inflow
 std::cout << "inflow: " << inflow_start << " , " << inflow_last << std::endl;
 nelem_start=inflow_start;
 nelem_end=inflow_last;
 icount=0;
 for (n=nelem_start; n <= nelem_end; n++)
 {
   ipnts[icount]=n;
   icount=icount+1;
 }
 if (icount > maxcount)
 {
   printf("\nError. Need to increase maxcount to at least %i\n",icount);
   exit(0);
 }
 CGNS_ENUMT( PointSetType_t ) bc_type = CGNS_ENUMV( ElementList );
/* write boundary conditions for ilo face */
 CALL_CGNS(cg_boco_write(index_file,index_base,index_zone,"inflow",CGNS_ENUMV( BCTunnelInflow ),bc_type, \
               icount,ipnts,&index_bc));
 std::cout << "\nSuccessfully wrote BC inflow (ElementList = "<<to_str<int>(CGNS_ENUMV( ElementList ))<<") to grid_c.cgns"<<std::endl;

 // BC outflow
 /* we know that for the unstructured zone, the following face elements */
 /* have been defined as outflow (real working code would check!): */
 std::cout << "outflow: " << outflow_start << " , " << outflow_last << std::endl;
 nelem_start=outflow_start;
 nelem_end=outflow_last;
 icount=0;
 for (n=nelem_start; n <= nelem_end; n++)
 {
   ipnts[icount]=n;
   icount=icount+1;
 }
 if (icount > maxcount)
 {
   printf("\nError. Need to increase maxcount to at least %i\n",icount);
   exit(0);
 }
 /* write boundary conditions for ihi face */
 CALL_CGNS(cg_boco_write(index_file,index_base,index_zone,"outflow",CGNS_ENUMV( BCExtrapolate ),bc_type,icount,ipnts,&index_bc));
 std::cout << "\nSuccessfully wrote BC outflow (ElementList = "<<to_str<int>(CGNS_ENUMV( ElementList ))<<") to grid_c.cgns"<<std::endl;


/* we know that for the unstructured zone, the following face elements */
/* have been defined as walls (real working code would check!): */
 std::cout << "side_walls: " << side_wall_start << " , " << side_wall_last << std::endl;
 nelem_start=side_wall_start;
 nelem_end=side_wall_last;
 icount=0;
 for (n=nelem_start; n <= nelem_end; n++)
 {
   ipnts[icount]=n;
   icount=icount+1;
 }
 if (icount > maxcount)
 {
   printf("\nError. Need to increase maxcount to at least %i\n",icount);
   exit(0);
 }
 /* write boundary conditions for wall faces */
 CALL_CGNS(cg_boco_write(index_file,index_base,index_zone,"Walls",CGNS_ENUMV( BCWallInviscid ),bc_type,icount,ipnts,&index_bc));
 std::cout << "\nSuccessfully wrote BC Walls (ElementList = "<<to_str<int>(CGNS_ENUMV( ElementList ))<<") to grid_c.cgns"<<std::endl;

/* ---------------------------------------------------------- */


/* close CGNS file */
 CALL_CGNS(cg_close(index_file));
 std::cout << "\nSuccessfully wrote unstructured grid to file grid_c.cgns"<< std::endl;

 BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( WriteStructured )
{
  /*
   dimension statements (note that tri-dimensional arrays
   x,y,z must be dimensioned exactly as [N][17][21] (N>=9)
   for this particular case or else they will be written to
   the CGNS file incorrectly!  Other options are to use 1-D
   arrays, use dynamic memory, or pass index values to a
   subroutine and dimension exactly there):
   */
  double x1[2][3][5],y1[2][3][5],z1[2][3][5];
  double x2[2][3][5],y2[2][3][5],z2[2][3][5];
  cgsize_t isize[3][3], ipnts[2][3];
  int ni,nj,nk,i,j,k;
  int index_file,icelldim,iphysdim,index_base;
  int index_zone,index_coord,index_bc;
  char basename[33],zonename[33];
  int ilo,ihi,jlo,jhi,klo,khi;

  /* create gridpoints for simple example: */
  ni=5;
  nj=3;
  nk=2;
  for (k=0; k < nk; ++k)
  {
    for (j=0; j < nj; ++j)
    {
      for (i=0; i < ni; ++i)
      {
        x1[k][j][i]=i;
        y1[k][j][i]=j;
        z1[k][j][i]=k;
        x2[k][j][i]=x1[k][j][i]+4.;
        y2[k][j][i]=y1[k][j][i];
        z2[k][j][i]=z1[k][j][i];
      }
    }
  }
  printf("\ncreated simple 3-D grid points (2 zones)");

  /*  WRITE X, Y, Z GRID POINTS TO CGNS FILE */
  /*  open CGNS file for write */
  cg_open("grid_str_2zones.cgns",CG_MODE_WRITE,&index_file);
  /*  create base (user can give any name) */
  strcpy(basename,"Base");
  icelldim=3;
  iphysdim=3;
  cg_base_write(index_file,basename,icelldim,iphysdim,&index_base);
  /*  vertex size */
  isize[0][0]=5;
  isize[0][1]=3;
  isize[0][2]=2;
  /*  cell size */
  isize[1][0]=isize[0][0]-1;
  isize[1][1]=isize[0][1]-1;
  isize[1][2]=isize[0][2]-1;
  /*  boundary vertex size (always zero for structured grids) */
  isize[2][0]=0;
  isize[2][1]=0;
  isize[2][2]=0;
  /*  define zone 1 name (user can give any name) */
  strcpy(zonename,"Zone 1");
  /*  create zone */
  cg_zone_write(index_file,index_base,zonename,*isize,CGNS_ENUMV( Structured ),&index_zone);
  /*  write grid coordinates (user must use SIDS-standard names here) */
  cg_coord_write(index_file,index_base,index_zone,CGNS_ENUMV( RealDouble ),"CoordinateX",x1,&index_coord);
  cg_coord_write(index_file,index_base,index_zone,CGNS_ENUMV( RealDouble ),"CoordinateY",y1,&index_coord);
  cg_coord_write(index_file,index_base,index_zone,CGNS_ENUMV( RealDouble ),"CoordinateZ",z1,&index_coord);

  ilo=1;
  ihi=isize[0][0];
  jlo=1;
  jhi=isize[0][1];
  klo=1;
  khi=isize[0][2];
  /* write boundary conditions for ilo face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jlo;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ilo;
  ipnts[1][1]=jhi;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Ilo",CGNS_ENUMV( BCTunnelInflow ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for ihi face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ihi;
  ipnts[0][1]=jlo;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jhi;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Ihi",CGNS_ENUMV( BCExtrapolate ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for jlo face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jlo;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jlo;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Jlo",CGNS_ENUMV( BCWallInviscid ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for jhi face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jhi;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jhi;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Jhi",CGNS_ENUMV( BCWallInviscid ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for klo face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jlo;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jhi;
  ipnts[1][2]=klo;
  cg_boco_write(index_file,index_base,index_zone,"Klo",CGNS_ENUMV( BCWallInviscid ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for khi face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jlo;
  ipnts[0][2]=khi;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jhi;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Khi",CGNS_ENUMV( BCWallInviscid ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);


  /*  define zone 2 name (user can give any name) */
  strcpy(zonename,"Zone 2");
  /*  create zone */
  cg_zone_write(index_file,index_base,zonename,*isize,CGNS_ENUMV( Structured ),&index_zone);
  /*  write grid coordinates (user must use SIDS-standard names here) */
  cg_coord_write(index_file,index_base,index_zone,CGNS_ENUMV( RealDouble ),"CoordinateX",x2,&index_coord);
  cg_coord_write(index_file,index_base,index_zone,CGNS_ENUMV( RealDouble ),"CoordinateY",y2,&index_coord);
  cg_coord_write(index_file,index_base,index_zone,CGNS_ENUMV( RealDouble ),"CoordinateZ",z2,&index_coord);


  ilo=1;
  ihi=isize[0][0];
  jlo=1;
  jhi=isize[0][1];
  klo=1;
  khi=isize[0][2];
  /* write boundary conditions for ilo face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jlo;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ilo;
  ipnts[1][1]=jhi;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Ilo",CGNS_ENUMV( BCTunnelInflow ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for ihi face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ihi;
  ipnts[0][1]=jlo;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jhi;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Ihi",CGNS_ENUMV( BCExtrapolate ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for jlo face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jlo;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jlo;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Jlo",CGNS_ENUMV( BCWallInviscid ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for jhi face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jhi;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jhi;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Jhi",CGNS_ENUMV( BCWallInviscid ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for klo face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jlo;
  ipnts[0][2]=klo;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jhi;
  ipnts[1][2]=klo;
  cg_boco_write(index_file,index_base,index_zone,"Klo",CGNS_ENUMV( BCWallInviscid ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* write boundary conditions for khi face, defining range first */
  /* (user can give any name) */
  /* lower point of range */
  ipnts[0][0]=ilo;
  ipnts[0][1]=jlo;
  ipnts[0][2]=khi;
  /* upper point of range */
  ipnts[1][0]=ihi;
  ipnts[1][1]=jhi;
  ipnts[1][2]=khi;
  cg_boco_write(index_file,index_base,index_zone,"Khi",CGNS_ENUMV( BCWallInviscid ),CGNS_ENUMV( PointRange ),2,ipnts[0],&index_bc);
  /* close CGNS file */
  cg_close(index_file);
  printf("\nSuccessfully added BCs (PointRange) to file grid_str_2zones.cgns\n");

  /*  close CGNS file */
  cg_close(index_file);
  printf("\nSuccessfully wrote grid to file grid_str_2zones.cgns\n");
  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ReadUnstructured )
{
  Core::instance().environment().options().set("log_level",(Uint)DEBUG);
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.CGNS.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("grid_c");
  meshreader->options().set("zone_handling",true);
  BOOST_CHECK_NO_THROW(meshreader->read_mesh_into("grid_c.cgns",mesh));

  // Write to gmsh
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  BOOST_CHECK_NO_THROW(gmsh_writer->write_from_to(mesh,"grid_c.msh"));

  // std::cout << mesh.tree() << std::endl;
  // std::cout << *mesh.access_component_checked("topology/Walls/elements_cf3.mesh.LagrangeP1.Quad3D/spaces/geometry/connectivity")->handle<Connectivity>() << std::endl;
   
  // Write to neu
  boost::shared_ptr< MeshWriter > neu_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.neu.Writer","meshwriter");
  BOOST_CHECK_NO_THROW(neu_writer->write_from_to(mesh,"grid_c.neu"));

  // Read from neu
  boost::shared_ptr< MeshReader > neu_reader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  Mesh& mesh_from_neu = *Core::instance().root().create_component<Mesh>("mesh_from_neu");
  BOOST_CHECK_NO_THROW(neu_reader->read_mesh_into("grid_c.neu",mesh_from_neu));

  // Write to gmsh
  BOOST_CHECK_NO_THROW(gmsh_writer->write_from_to(mesh_from_neu,"cgns2neu2gmsh.msh"));

  //CFinfo << mesh_from_neu->tree() << CFendl;
  BOOST_CHECK(true);
}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ReadCGNS_Structured )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.CGNS.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("grid_str_2zones");
  meshreader->read_mesh_into("grid_str_2zones.cgns",mesh);

  boost::shared_ptr< MeshTransformer > info = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.Info", "info");
  info->transform(mesh);
  // Write to gmsh
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,"grid_str_2zones.msh");

  // Write to neu
  boost::shared_ptr< MeshWriter > neu_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.neu.Writer","meshwriter");
  neu_writer->write_from_to(mesh,"grid_str_2zones.neu");

  // Read from neu
  boost::shared_ptr< MeshReader > neu_reader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  Mesh& mesh_from_neu = *Core::instance().root().create_component<Mesh>("grid_str_2zones_from_neu");
  neu_reader->read_mesh_into("grid_str_2zones.neu",mesh_from_neu);

  // Write to gmsh
  gmsh_writer->write_from_to(mesh_from_neu,"cgns2neu2gmsh_str_2zones.msh");


//  CFinfo << mesh_from_neu.tree() << CFendl;
  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( WriteCNGS_unstructured )
{
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.CGNS.Reader","meshreader");



  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("grid_c_unstr");
  meshreader->read_mesh_into("grid_c.cgns",mesh);

  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.CGNS.Writer","meshwriter");

  meshwriter->write_from_to(mesh,"grid_c2cgns.cgns");

  Mesh& mesh2 = *Core::instance().root().create_component<Mesh>("grid_c2cgns");
  meshreader->read_mesh_into("grid_c2cgns.cgns",mesh2);

  boost::shared_ptr< MeshTransformer > info = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.Info", "info");
  //info->transform(mesh2);

  // Write to gmsh
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh2,"grid_c2cgns2gmsh.msh");
  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( WriteCNGS_mixed )
{
  boost::shared_ptr< MeshReader > neu_reader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("quadtriag_mixed");
  neu_reader->read_mesh_into("../../resources/quadtriag.neu",mesh);

  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.CGNS.Writer","meshwriter");

  meshwriter->write_from_to(mesh,"quadtriag2cgns.cgns");

  boost::shared_ptr< MeshReader > cgns_reader = build_component_abstract_type<MeshReader>("cf3.mesh.CGNS.Reader","meshreader");

  Mesh& mesh2 = *Core::instance().root().create_component<Mesh>("quadtriag2cgns");
  cgns_reader->read_mesh_into("quadtriag2cgns.cgns",mesh2);

  boost::shared_ptr< MeshTransformer > info = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.Info", "info");
  info->transform(mesh2);

  // Write to gmsh
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh2,"quadtriag2cgns2gmsh.msh");
  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

