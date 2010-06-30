#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/OSystem.hpp"
#include "Common/LibLoader.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/CGNS/Shared.hpp"


using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::CGNS;

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
  rapidxml::xml_document<> doc;    // character type defaults to char
  char* ctext;

  rapidxml::xml_node<>* parsed_config()
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
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CGNS","meshreader");
  BOOST_CHECK_EQUAL(meshreader->name(),"meshreader");
  BOOST_CHECK_EQUAL(meshreader->get_format(),"CGNS");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestCGNSLib )
{

 int maxelemi = 20*16*8;
 double x[21*17*9],y[21*17*9],z[21*17*9];
 int isize[3][1],ielem[maxelemi][8];
 int ni,nj,nk,iset,i,j,k,index_file,icelldim,iphysdim;
 int index_base,index_zone,index_coord,ielem_no,nelem_start;
 int ifirstnode,nelem_end,nbdyelem,index_section;
 char basename[33],zonename[33];

/* create gridpoints for simple example: */
 ni=21;
 nj=17;
 nk=9;
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
 strcpy(zonename,"Zone  1");
/* vertex size */
 isize[0][0]=ni*nj*nk;
/* cell size */
 isize[1][0]=(ni-1)*(nj-1)*(nk-1);
/* boundary vertex size (zero if elements not sorted) */
 isize[2][0]=0;
/* create zone */
 CALL_CGNS(cg_zone_write(index_file,index_base,zonename,isize[0],Unstructured,&index_zone));
/* write grid coordinates (user must use SIDS-standard names here) */
 CALL_CGNS(cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateX",x,&index_coord));
 CALL_CGNS(cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateY",y,&index_coord));
 CALL_CGNS(cg_coord_write(index_file,index_base,index_zone,RealDouble,"CoordinateZ",z,&index_coord));
/* set element connectivity: */
/* ---------------------------------------------------------- */
/* do all the HEXA_8 elements (this part is mandatory): */
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
/* unsorted boundary elements */
 nbdyelem=0;
/* write HEXA_8 element connectivity (user can give any name) */
 CALL_CGNS(cg_section_write(index_file,index_base,index_zone,"Elem",HEXA_8,nelem_start,  \
                  nelem_end,nbdyelem,ielem[0],&index_section));
/* ---------------------------------------------------------- */
int maxelemj = 1216;
int jelem[maxelemj][4];
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
 CALL_CGNS(cg_section_write(index_file,index_base,index_zone,"InflowElem",QUAD_4,nelem_start,  \
                  nelem_end,nbdyelem,jelem[0],&index_section));
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
 CALL_CGNS(cg_section_write(index_file,index_base,index_zone,"OutflowElem",QUAD_4,nelem_start,  \
                  nelem_end,nbdyelem,jelem[0],&index_section));
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
 CALL_CGNS(cg_section_write(index_file,index_base,index_zone,"SidewallElem",QUAD_4,nelem_start,  \
                  nelem_end,nbdyelem,jelem[0],&index_section));
/* ---------------------------------------------------------- */

 // part 2: add boundary conditions
 int icount, n, index_bc;
 int maxcount(960);
 int ipnts[maxcount];
 // BC inflow
 nelem_start=2561;
 nelem_end=2688;
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
/* write boundary conditions for ilo face */
 CALL_CGNS(cg_boco_write(index_file,index_base,index_zone,"inflow",BCTunnelInflow,ElementList, \
               icount,ipnts,&index_bc));

 // BC outflow
 /* we know that for the unstructured zone, the following face elements */
 /* have been defined as outflow (real working code would check!): */
 nelem_start=2689;
 nelem_end=2816;
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
 CALL_CGNS(cg_boco_write(index_file,index_base,index_zone,"outflow",BCExtrapolate,ElementList,icount,ipnts,&index_bc));


/* we know that for the unstructured zone, the following face elements */
/* have been defined as walls (real working code would check!): */
 nelem_start=2817;
 nelem_end=3776;
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
 CALL_CGNS(cg_boco_write(index_file,index_base,index_zone,"Walls",BCWallInviscid,ElementList,icount,ipnts,&index_bc));


/* ---------------------------------------------------------- */


/* close CGNS file */
 CALL_CGNS(cg_close(index_file));
 //printf("\nSuccessfully wrote unstructured grid to file grid_c.cgns\n");

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ReadCGNS )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CGNS","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("grid_c.cgns");

  // the mesh to store in
  CMesh::Ptr mesh = meshreader->create_mesh_from(fp_in);

  // CFinfo << mesh->tree() << CFendl;

  // Write to Gmsh
  boost::filesystem::path fp_out ("grid_c.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);

  // Write to Neu
  boost::filesystem::path neu_out ("grid_c.neu");
  CMeshWriter::Ptr neu_writer = create_component_abstract_type<CMeshWriter>("Neu","meshwriter");
  neu_writer->write_from_to(mesh,neu_out);

  // Read from Neu
  CMeshReader::Ptr neu_reader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  CMesh::Ptr mesh_from_neu = neu_reader->create_mesh_from(neu_out);

  // Write to Gmsh
  boost::filesystem::path gmsh_out ("cgns2neu2gmsh.msh");
  gmsh_writer->write_from_to(mesh_from_neu,gmsh_out);
  
  
//  CFinfo << mesh_from_neu->tree() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

