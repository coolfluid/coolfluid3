// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// this file is en-block included into utest-parallel-collective.cpp
// do not include anything here, rather in utest-parallel-collective.cpp

////////////////////////////////////////////////////////////////////////////////

struct PEBroadcastFixture
{
  /// common setup for each test case
  PEBroadcastFixture()
  {
    int i;

    // rank and proc
    nproc=PE::Comm::instance().size();
    irank=PE::Comm::instance().rank();

    // ptr helpers
    sndcnt=0;
    rcvcnt=0;
    ptr_snddat=new double[nproc];
    ptr_rcvdat=new double[nproc*nproc];
    ptr_sndmap=new int[nproc];
    ptr_rcvmap=new int[nproc];
    ptr_tmprcv=new double[nproc];

    // std::Vector helpers
    vec_snddat.resize(nproc);
    vec_rcvdat.resize(nproc*nproc);
    vec_sndmap.resize(nproc);
    vec_rcvmap.resize(nproc);
    vec_tmprcv.resize(0);
    vec_tmprcvchr.resize(nproc*sizeof(double));
    vec_snddatchr.resize(nproc*sizeof(double));
  }

  /// common tear-down for each test case
  ~PEBroadcastFixture()
  {
    delete[] ptr_snddat;
    delete[] ptr_rcvdat;
    delete[] ptr_sndmap;
    delete[] ptr_rcvmap;
    delete[] ptr_tmprcv;
  }

  /// number of processes
  int nproc;
  /// rank of process
  int irank;

  /// data for raw pointers
  int     sndcnt;
  int     rcvcnt;
  double* ptr_snddat;
  double* ptr_rcvdat;
  int*    ptr_sndmap;
  int*    ptr_rcvmap;
  double* ptr_tmprcv;

  /// data for std::vectors
  std::vector<double> vec_snddat;
  std::vector<double> vec_rcvdat;
  std::vector<int>    vec_sndmap;
  std::vector<int>    vec_rcvmap;
  std::vector<double> vec_tmprcv;
  std::vector<char>   vec_tmprcvchr;
  std::vector<char>   vec_snddatchr;

  /// helper function for constant size data - setting up input and verification data
  void setup_data_constant()
  {
    int i,j;
    for (i=0; i<nproc; i++){
      ptr_snddat[i]=(irank+1)*1000+(i+1);
      for (j=0; j<nproc; j++) ptr_rcvdat[i*nproc+j]=(i+1)*1000+(j+1);
    }
    sndcnt=nproc;
    rcvcnt=nproc;
    vec_snddat.assign(ptr_snddat,ptr_snddat+nproc);
    vec_rcvdat.assign(ptr_rcvdat,ptr_rcvdat+nproc*nproc);
    vec_snddatchr.assign((char*)(ptr_snddat),(char*)(ptr_snddat+nproc));
  }

  /// helper function for variable size data - setting up input and verification data
  void setup_data_variable()
  {
    int i,j,k,l;
    sndcnt=nproc/2;
    rcvcnt=nproc/2;
    for(i=0; i<nproc; i++) { // making debugger shut up for uninitialized values
      ptr_snddat[i]=0.;
      ptr_sndmap[i]=0;
      ptr_rcvmap[i]=0;
    }
    for(i=0; i<nproc; i++) for(j=0; j<nproc; j++) ptr_rcvdat[i*nproc+j]=0; // making debugger shut up for uninitialized values
    for (i=0; i<nproc; i++){
      ptr_snddat[i]=(irank+1)*1000+(i+1);
      for (j=0; j<nproc; j++) ptr_rcvdat[i*nproc+j]=(i+1)*1000+(2*(rcvcnt-j)-1);
    }
    for (i=0; i<sndcnt; i++)
      ptr_sndmap[i]=2*i; // every second
    for (i=0; i<rcvcnt; i++)
      ptr_rcvmap[i]=rcvcnt-1-i; // inverse into contiguous
    vec_snddat.assign(ptr_snddat,ptr_snddat+nproc);
    vec_rcvdat.assign(ptr_rcvdat,ptr_rcvdat+nproc*nproc);
    vec_snddatchr.assign((char*)(ptr_snddat),(char*)(ptr_snddat+nproc));
    vec_sndmap.assign(ptr_sndmap,ptr_sndmap+sndcnt);
    vec_rcvmap.assign(ptr_rcvmap,ptr_rcvmap+rcvcnt);
  }

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PEBroadcastSuite, PEBroadcastFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Broadcast )
{
  PEProcessSortedExecute(-1,CFinfo << "Testing broadcast " << irank << "/" << nproc << CFendl; );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( broadcast_ptr_constant )
{
  int i,r;

  setup_data_constant();

  for (r=0; r<nproc; r++) {

    delete[] ptr_tmprcv;
    ptr_tmprcv=0;

    ptr_tmprcv=PE::Comm::instance().broadcast(ptr_snddat, nproc, (double*)0, r);
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[r*nproc+i] );

    for (i=0; i<nproc; i++) ptr_tmprcv[i]=0.;
    PE::Comm::instance().broadcast(ptr_snddat, nproc, ptr_tmprcv, r);
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[r*nproc+i] );

    for (i=0; i<nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
    PE::Comm::instance().broadcast(ptr_tmprcv, nproc, ptr_tmprcv, r);
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[r*nproc+i] );

    delete[] ptr_tmprcv;
    ptr_tmprcv=0;

    ptr_tmprcv=(double*)PE::Comm::instance().broadcast((char*)ptr_snddat, nproc, (char*)0, r, sizeof(double));
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[r*nproc+i] );

    for (i=0; i<nproc; i++) ptr_tmprcv[i]=0.;
    PE::Comm::instance().broadcast((char*)ptr_snddat, nproc, (char*)ptr_tmprcv, r, sizeof(double));
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[r*nproc+i] );

    for (i=0; i<nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
    PE::Comm::instance().broadcast((char*)ptr_tmprcv, nproc, (char*)ptr_tmprcv, r, sizeof(double));
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[r*nproc+i] );
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( broadcast_vector_constant )
{
  int i,r;

  setup_data_constant();

  for (r=0; r<nproc; r++) {

    vec_tmprcv.resize(0);
    vec_tmprcv.reserve(0);
    PE::Comm::instance().broadcast(vec_snddat, vec_tmprcv, r);
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[r*nproc+i] );
    BOOST_CHECK_EQUAL( (int)vec_tmprcv.size() , rcvcnt );

    vec_tmprcv.assign(nproc,0.);
    PE::Comm::instance().broadcast(vec_snddat, vec_tmprcv, r);
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[r*nproc+i] );

    vec_tmprcv=vec_snddat;
    PE::Comm::instance().broadcast(vec_tmprcv, vec_tmprcv, r);
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[r*nproc+i] );

    vec_tmprcvchr.resize(0);
    vec_tmprcvchr.reserve(0);
    PE::Comm::instance().broadcast(vec_snddatchr, vec_tmprcvchr, r );
    BOOST_CHECK_EQUAL( vec_tmprcvchr.size() , sizeof(double)*rcvcnt );
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[r*nproc+i] );

    for (i=0; i<nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=0.;
    PE::Comm::instance().broadcast(vec_snddatchr, vec_tmprcvchr, r );
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[r*nproc+i] );

    vec_tmprcvchr.assign((char*)(ptr_snddat),(char*)(ptr_snddat+nproc));
    PE::Comm::instance().broadcast(vec_tmprcvchr, vec_tmprcvchr, r );
    for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[r*nproc+i] );
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( broadcast_ptr_variable )
{

  int i,j,k,r;

  setup_data_variable();

  for (r=0; r<nproc; r++) {

    delete[] ptr_tmprcv;
    ptr_tmprcv=0;

    ptr_tmprcv=PE::Comm::instance().broadcast(ptr_snddat, sndcnt, ptr_sndmap, (double*)0, ptr_rcvmap, r);
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[nproc*r+i] );

    for (i=0; i<sndcnt; i++) ptr_tmprcv[i]=0.;
    PE::Comm::instance().broadcast(ptr_snddat, sndcnt, ptr_sndmap, ptr_tmprcv, ptr_rcvmap, r);
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[nproc*r+i] );

    delete[] ptr_tmprcv;
    ptr_tmprcv=new double[nproc];
    for (i=0; i<nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
    PE::Comm::instance().broadcast(ptr_tmprcv, sndcnt, ptr_sndmap, ptr_tmprcv, ptr_rcvmap, r);
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[nproc*r+i] );

    delete[] ptr_tmprcv;
    ptr_tmprcv=0;
    ptr_tmprcv=(double*)PE::Comm::instance().broadcast((char*)ptr_snddat, sndcnt, ptr_sndmap, (char*)0, ptr_rcvmap, r, sizeof(double));
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[nproc*r+i] );

    for (i=0; i<sndcnt; i++) ptr_tmprcv[i]=0.;
    PE::Comm::instance().broadcast((char*)ptr_snddat, sndcnt, ptr_sndmap, (char*)ptr_tmprcv, ptr_rcvmap, r, sizeof(double));
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[nproc*r+i] );

    delete[] ptr_tmprcv;
    ptr_tmprcv=new double[nproc];
    for (i=0; i<nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
    PE::Comm::instance().broadcast((char*)ptr_tmprcv, sndcnt, ptr_sndmap, (char*)ptr_tmprcv, ptr_rcvmap, r, sizeof(double));
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[nproc*r+i] );
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( broadcast_vector_variable )
{
  int i,j,k,r;

  setup_data_variable();

  for (r=0; r<nproc; r++) {

    vec_tmprcv.resize(0);
    vec_tmprcv.reserve(0);
    PE::Comm::instance().broadcast(vec_snddat, vec_sndmap, vec_tmprcv, vec_rcvmap, r);
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[nproc*r+i] );

    for (i=0; i<sndcnt; i++) vec_tmprcv[i]=0.;
    PE::Comm::instance().broadcast(vec_snddat, vec_sndmap, vec_tmprcv, vec_rcvmap, r);
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[nproc*r+i] );

    vec_tmprcv.resize(nproc);
    vec_tmprcv.reserve(nproc);
    for (i=0; i<nproc; i++) vec_tmprcv[i]=vec_snddat[i];
    PE::Comm::instance().broadcast(vec_tmprcv, vec_sndmap, vec_tmprcv, vec_rcvmap, r);
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[nproc*r+i] );

    vec_tmprcvchr.resize(0);
    vec_tmprcvchr.reserve(0);
    PE::Comm::instance().broadcast(vec_snddatchr, vec_sndmap, vec_tmprcvchr, vec_rcvmap, r, sizeof(double));
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( ((double*)&vec_tmprcvchr[0])[i] , vec_rcvdat[nproc*r+i] );

    for (i=0; i<nproc; i++) ((double*)&vec_tmprcvchr[0])[i]=0.;
    PE::Comm::instance().broadcast(vec_snddatchr, vec_sndmap, vec_tmprcvchr, vec_rcvmap, r, sizeof(double));
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( ((double*)&vec_tmprcvchr[0])[i] , vec_rcvdat[nproc*r+i] );

    vec_tmprcvchr.resize(nproc*sizeof(double));
    vec_tmprcvchr.reserve(nproc*sizeof(double));
    for (i=0; i<nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=vec_snddat[i];
    PE::Comm::instance().broadcast(vec_tmprcvchr, vec_sndmap, vec_tmprcvchr, vec_rcvmap, r, sizeof(double));
    for (i=0; i<sndcnt; i++) BOOST_CHECK_EQUAL( ((double*)&vec_tmprcvchr[0])[i] , vec_rcvdat[nproc*r+i] );
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

