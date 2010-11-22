// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//
// IMPORTANT:
// run it both on 1 and many cores
// for example: mpirun -np 4 ./test-parallel-environment --report_level=confirm or --report_level=detailed

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Common 's parallel environment - part of testing collective communications."

////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>

////////////////////////////////////////////////////////////////////////////////

#include "Common/MPI/PE.hpp"
#include "Common/MPI/all_to_all.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct PECollectiveFixture
{
  /// common setup for each test case
  PECollectiveFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~PECollectiveFixture() { }

  /// common data
  int m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PECollectiveSuite, PECollectiveFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PE::instance().is_init() , true );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_to_all )
{
  int i,j,k,l;
  const int nproc=PE::instance().size();
  const int irank=PE::instance().rank();

  // data
  int sndcnt=0;
  int rcvcnt=0;
  int* ptr_sndcnt=new int[nproc];
  int* ptr_rcvcnt=new int[nproc];
  double* ptr_snddat=new double[nproc*nproc];
  double* ptr_rcvdat=new double[nproc*nproc];
  int* ptr_sndmap=new int[nproc*nproc];
  int* ptr_rcvmap=new int[nproc*nproc];
  double* ptr_tmprcv=new double[nproc*nproc];
  int* ptr_tmpcnt=new int[nproc];
  std::vector<int> vec_sndcnt(nproc);
  std::vector<int> vec_rcvcnt(nproc);
  std::vector<double> vec_snddat(nproc*nproc);
  std::vector<double> vec_rcvdat(nproc*nproc);
  std::vector<int> vec_sndmap(nproc*nproc);
  std::vector<int> vec_rcvmap(nproc*nproc);
  std::vector<double> vec_tmprcv(0);
  std::vector<int> vec_tmpcnt(nproc);
  std::vector<char> vec_tmprcvchr(nproc*nproc);
  std::vector<char> vec_snddatchr(nproc*nproc);

  // constant size - setting up input and verification data
  for (i=0; i<nproc; i++)
    for (j=0; j<nproc; j++){
      ptr_snddat[i*nproc+j]=(irank+1)*1000000+(i+1)*10000+(j+1);
      ptr_rcvdat[i*nproc+j]=(i+1)*1000000+(irank+1)*10000+(j+1);
    }
  sndcnt=nproc*nproc;
  rcvcnt=nproc*nproc;
  vec_snddat.assign(ptr_snddat,ptr_snddat+nproc*nproc);
  vec_rcvdat.assign(ptr_rcvdat,ptr_rcvdat+nproc*nproc);
  vec_snddatchr.assign((char*)(ptr_snddat),(char*)(ptr_snddat+nproc*nproc));

  // constant size - pointers
  delete[] ptr_tmprcv;
  ptr_tmprcv=0;

  ptr_tmprcv=mpi::all_to_all(PE::instance(), ptr_snddat, nproc, (double*)0);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=0.;
  mpi::all_to_all(PE::instance(), ptr_snddat, nproc, ptr_tmprcv);
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  mpi::all_to_all(PE::instance(), ptr_tmprcv, nproc, ptr_tmprcv);
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=(double*)mpi::all_to_all(PE::instance(), (char*)ptr_snddat, nproc, (char*)0, sizeof(double));
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=0.;
  mpi::all_to_all(PE::instance(), (char*)ptr_snddat, nproc, (char*)ptr_tmprcv, sizeof(double));
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  mpi::all_to_all(PE::instance(), (char*)ptr_tmprcv, nproc, (char*)ptr_tmprcv, sizeof(double));
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );


  // constant size - std vectors
  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);
  mpi::all_to_all(PE::instance(), vec_snddat, vec_tmprcv);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[i] );
  BOOST_CHECK_EQUAL( (int)vec_tmprcv.size() , rcvcnt );

  vec_tmprcv.assign(nproc*nproc,0.);
  mpi::all_to_all(PE::instance(), vec_snddat, vec_tmprcv);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[i] );

  vec_tmprcv=vec_snddat;
  mpi::all_to_all(PE::instance(), vec_tmprcv, vec_tmprcv);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[i] );

  vec_tmprcvchr.resize(0);
  vec_tmprcvchr.reserve(0);
  mpi::all_to_all(PE::instance(), vec_snddatchr, vec_tmprcvchr );
  BOOST_CHECK_EQUAL( vec_tmprcvchr.size() , sizeof(double)*rcvcnt );
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=0.;
  mpi::all_to_all(PE::instance(), vec_snddatchr, vec_tmprcvchr );
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[i] );

  vec_tmprcvchr.assign((char*)(ptr_snddat),(char*)(ptr_snddat+nproc*nproc));
  mpi::all_to_all(PE::instance(), vec_tmprcvchr, vec_tmprcvchr );
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[i] );

  // variable size - setting up input and verification data
  for (i=0; i<nproc; i++){
    ptr_sndcnt[i]=(i+irank*irank)%nproc;
    ptr_rcvcnt[i]=(i*i+irank)%nproc;
  }
  for(i=0; i<nproc*nproc; i++) { // making debugger shut up for uninitialized values
    ptr_snddat[i]=0.;
    ptr_rcvdat[i]=0.;
    ptr_sndmap[i]=0;
    ptr_rcvmap[i]=0;
  }
  for(i=0, k=0; i<nproc; i++)
    for(j=0; j<ptr_sndcnt[i]; j++, k++)
      ptr_snddat[k]=(irank+1)*1000000+(i+1)*10000+(j+1);
  for(i=0, k=0; i<nproc; i++)
    for(j=0; j<ptr_rcvcnt[i]; j++, k++)
      ptr_rcvdat[k]=(i+1)*1000000+(irank+1)*10000+(j+1);
  for (i=0, k=0,l=0; i<nproc; k+=ptr_sndcnt[i], i++)
    for (j=0; j<ptr_sndcnt[i]; j++,l++)
      ptr_sndmap[l]=k+ptr_sndcnt[i]-1-j; // flipping all sets for each process
  for (i=0, k=0, l=0; i<nproc; k+=ptr_rcvcnt[i], i++)
    for (j=0; j<ptr_rcvcnt[i]; j++, l++)
      ptr_rcvmap[l]=i*nproc+ptr_rcvcnt[i]-1-j; // redirecting to align start with nproc numbers
  for(i=0, sndcnt=0, rcvcnt=0; i<nproc; i++){
    sndcnt+=ptr_sndcnt[i];
    rcvcnt+=ptr_rcvcnt[i];
  }
  vec_sndcnt.assign(ptr_sndcnt,ptr_sndcnt+nproc);
  vec_rcvcnt.assign(ptr_rcvcnt,ptr_rcvcnt+nproc);
  vec_snddat.assign(ptr_snddat,ptr_snddat+nproc*nproc);
  vec_rcvdat.assign(ptr_rcvdat,ptr_rcvdat+nproc*nproc);
  vec_sndmap.assign(ptr_sndmap,ptr_sndmap+nproc*nproc);
  vec_rcvmap.assign(ptr_rcvmap,ptr_rcvmap+nproc*nproc);
  vec_snddatchr.assign((char*)(ptr_snddat),(char*)(ptr_snddat+nproc*nproc));

  // variable size - pointers
  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=mpi::all_to_all(PE::instance(), ptr_snddat, ptr_sndcnt, (double*)0, ptr_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[k]=0.;
  mpi::all_to_all(PE::instance(), ptr_snddat, ptr_sndcnt, ptr_tmprcv, ptr_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  mpi::all_to_all(PE::instance(), ptr_tmprcv, ptr_sndcnt, ptr_tmprcv, ptr_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  ptr_tmprcv=mpi::all_to_all(PE::instance(), ptr_snddat, ptr_sndcnt, (double*)0, ptr_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[k]=0.;
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), ptr_snddat, ptr_sndcnt, ptr_tmprcv, ptr_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), ptr_tmprcv, ptr_sndcnt, ptr_tmprcv, ptr_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=mpi::all_to_all(PE::instance(), ptr_snddat, ptr_sndcnt, ptr_sndmap, (double*)0, ptr_rcvcnt, ptr_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[i*nproc+j]=0.;
  mpi::all_to_all(PE::instance(), ptr_snddat, ptr_sndcnt, ptr_sndmap, ptr_tmprcv, ptr_rcvcnt, ptr_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  mpi::all_to_all(PE::instance(), ptr_tmprcv, ptr_sndcnt, ptr_sndmap, ptr_tmprcv, ptr_rcvcnt, ptr_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=(double*)mpi::all_to_all(PE::instance(), (char*)ptr_snddat, ptr_sndcnt, (char*)0, ptr_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[k]=0.;
  mpi::all_to_all(PE::instance(), (char*)ptr_snddat, ptr_sndcnt, (char*)ptr_tmprcv, ptr_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  mpi::all_to_all(PE::instance(), (char*)ptr_tmprcv, ptr_sndcnt, (char*)ptr_tmprcv, ptr_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  ptr_tmprcv=(double*)mpi::all_to_all(PE::instance(), (char*)ptr_snddat, ptr_sndcnt, (char*)0, ptr_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[k]=0.;
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), (char*)ptr_snddat, ptr_sndcnt, (char*)ptr_tmprcv, ptr_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), (char*)ptr_tmprcv, ptr_sndcnt, (char*)ptr_tmprcv, ptr_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=(double*)mpi::all_to_all(PE::instance(), (char*)ptr_snddat, ptr_sndcnt, ptr_sndmap, (char*)0, ptr_rcvcnt, ptr_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[i*nproc+j]=0.;
  mpi::all_to_all(PE::instance(), (char*)ptr_snddat, ptr_sndcnt, ptr_sndmap, (char*)ptr_tmprcv, ptr_rcvcnt, ptr_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  mpi::all_to_all(PE::instance(), (char*)ptr_tmprcv, ptr_sndcnt, ptr_sndmap, (char*)ptr_tmprcv, ptr_rcvcnt, ptr_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init


  // variable size - std vectors
  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);
  mpi::all_to_all(PE::instance(), vec_snddat, vec_sndcnt, vec_tmprcv, vec_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) vec_tmprcv[k]=0.;
  mpi::all_to_all(PE::instance(), vec_snddat, vec_sndcnt, vec_tmprcv, vec_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  vec_tmprcv.resize(nproc*nproc);
  vec_tmprcv.reserve(nproc*nproc);
  for (i=0; i<nproc*nproc; i++) vec_tmprcv[i]=vec_snddat[i];
  mpi::all_to_all(PE::instance(), vec_tmprcv, vec_sndcnt, vec_tmprcv, vec_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), vec_snddat, vec_sndcnt, vec_tmprcv, vec_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) vec_tmprcv[k]=0.;
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), vec_snddat, vec_sndcnt, vec_tmprcv, vec_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  vec_tmprcv.resize(nproc*nproc);
  vec_tmprcv.reserve(nproc*nproc);
  for (i=0; i<nproc*nproc; i++) vec_tmprcv[i]=vec_snddat[i];
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), vec_tmprcv, vec_sndcnt, vec_tmprcv, vec_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);
  mpi::all_to_all(PE::instance(), vec_snddat, vec_sndcnt, vec_sndmap, vec_tmprcv, vec_rcvcnt, vec_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) vec_tmprcv[i*nproc+j]=0.;
  mpi::all_to_all(PE::instance(), vec_snddat, vec_sndcnt, vec_sndmap, vec_tmprcv, vec_rcvcnt, vec_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  vec_tmprcv.resize(nproc*nproc);
  vec_tmprcv.reserve(nproc*nproc);
  for (i=0; i<nproc*nproc; i++) vec_tmprcv[i]=vec_snddat[i];
  mpi::all_to_all(PE::instance(), vec_tmprcv, vec_sndcnt, vec_sndmap, vec_tmprcv, vec_rcvcnt, vec_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  vec_tmprcvchr.resize(0);
  vec_tmprcvchr.reserve(0);
  mpi::all_to_all(PE::instance(), vec_snddatchr, vec_sndcnt, vec_tmprcvchr, vec_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) ((double*)(&vec_tmprcvchr[0]))[k]=0.;
  mpi::all_to_all(PE::instance(), vec_snddatchr, vec_sndcnt, vec_tmprcvchr, vec_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  vec_tmprcv.resize(nproc*nproc);
  vec_tmprcv.reserve(nproc*nproc);
  for (i=0; i<nproc*nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=vec_snddat[i];
  mpi::all_to_all(PE::instance(), vec_tmprcvchr, vec_sndcnt, vec_tmprcvchr, vec_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  vec_tmprcvchr.resize(0);
  vec_tmprcvchr.reserve(0);
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), vec_snddatchr, vec_sndcnt, vec_tmprcvchr, vec_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) ((double*)(&vec_tmprcvchr[0]))[k]=0.;
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), vec_snddatchr, vec_sndcnt, vec_tmprcvchr, vec_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  vec_tmprcvchr.resize(nproc*nproc);
  vec_tmprcvchr.reserve(nproc*nproc);
  for (i=0; i<nproc*nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=vec_snddat[i];
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  mpi::all_to_all(PE::instance(), vec_tmprcvchr, vec_sndcnt, vec_tmprcvchr, vec_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  vec_tmprcvchr.resize(0);
  vec_tmprcvchr.reserve(0);
  mpi::all_to_all(PE::instance(), vec_snddatchr, vec_sndcnt, vec_sndmap, vec_tmprcvchr, vec_rcvcnt, vec_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) vec_tmprcvchr[i*nproc+j]=0.;
  mpi::all_to_all(PE::instance(), vec_snddatchr, vec_sndcnt, vec_sndmap, vec_tmprcvchr, vec_rcvcnt, vec_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  vec_tmprcvchr.resize(nproc*nproc);
  vec_tmprcvchr.reserve(nproc*nproc);
  for (i=0; i<nproc*nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=vec_snddat[i];
  mpi::all_to_all(PE::instance(), vec_tmprcvchr, vec_sndcnt, vec_sndmap, vec_tmprcvchr, vec_rcvcnt, vec_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  // free data
  delete[] ptr_sndcnt;
  delete[] ptr_rcvcnt;
  delete[] ptr_snddat;
  delete[] ptr_rcvdat;
  delete[] ptr_sndmap;
  delete[] ptr_rcvmap;
  delete[] ptr_tmprcv;
  delete[] ptr_tmpcnt;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize )
{
  PE::instance().finalize();
  BOOST_CHECK_EQUAL( PE::instance().is_init() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

