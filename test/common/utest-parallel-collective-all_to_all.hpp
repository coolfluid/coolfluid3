// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// this file is en-block included into utest-parallel-collective.cpp
// do not include anything here, rather in utest-parallel-collective.cpp

////////////////////////////////////////////////////////////////////////////////

struct PEAllToAllFixture
{
  /// common setup for each test case
  PEAllToAllFixture()
  {
    // rank and proc
    nproc=PE::Comm::instance().size();
    irank=PE::Comm::instance().rank();

    // ptr helpers
    sndcnt=0;
    rcvcnt=0;
    ptr_sndcnt=new int[nproc];
    ptr_rcvcnt=new int[nproc];
    ptr_snddat=new double[nproc*nproc];
    ptr_rcvdat=new double[nproc*nproc];
    ptr_sndmap=new int[nproc*nproc];
    ptr_rcvmap=new int[nproc*nproc];
    ptr_tmprcv=new double[nproc*nproc];
    ptr_tmpcnt=new int[nproc];

    // std::Vector helpers
    vec_sndcnt.resize(nproc);
    vec_rcvcnt.resize(nproc);
    vec_snddat.resize(nproc*nproc);
    vec_rcvdat.resize(nproc*nproc);
    vec_sndmap.resize(nproc*nproc);
    vec_rcvmap.resize(nproc*nproc);
    vec_tmprcv.resize(0);
    vec_tmpcnt.resize(nproc);
    vec_tmprcvchr.resize(nproc*nproc*sizeof(double));
    vec_snddatchr.resize(nproc*nproc*sizeof(double));
  }

  /// common tear-down for each test case
  ~PEAllToAllFixture()
  {
    delete[] ptr_sndcnt;
    delete[] ptr_rcvcnt;
    delete[] ptr_snddat;
    delete[] ptr_rcvdat;
    delete[] ptr_sndmap;
    delete[] ptr_rcvmap;
    delete[] ptr_tmprcv;
    delete[] ptr_tmpcnt;
  }

  /// number of processes
  int nproc;
  /// rank of process
  int irank;

  /// data for raw pointers
  int     sndcnt;
  int     rcvcnt;
  int*    ptr_sndcnt;
  int*    ptr_rcvcnt;
  double* ptr_snddat;
  double* ptr_rcvdat;
  int*    ptr_sndmap;
  int*    ptr_rcvmap;
  double* ptr_tmprcv;
  int*    ptr_tmpcnt;

  /// data for std::vectors
  std::vector<int>    vec_sndcnt;
  std::vector<int>    vec_rcvcnt;
  std::vector<double> vec_snddat;
  std::vector<double> vec_rcvdat;
  std::vector<int>    vec_sndmap;
  std::vector<int>    vec_rcvmap;
  std::vector<double> vec_tmprcv;
  std::vector<int>    vec_tmpcnt;
  std::vector<char>   vec_tmprcvchr;
  std::vector<char>   vec_snddatchr;

  /// helper function for constant size data - setting up input and verification data
  void setup_data_constant()
  {
    int i,j;
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
  }

  /// helper function for variable size data - setting up input and verification data
  void setup_data_variable()
  {
    int i,j,k,l;
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
  }

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PEAllToAllSuite, PEAllToAllFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_to_all )
{
  PEProcessSortedExecute(-1,CFinfo << "Testing all_to_all " << irank << "/" << nproc << CFendl; );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_to_all_ptr_constant )
{
  int i;

  setup_data_constant();

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;

  ptr_tmprcv=PE::Comm::instance().all_to_all(ptr_snddat, nproc, (double*)0);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=0.;
  PE::Comm::instance().all_to_all(ptr_snddat, nproc, ptr_tmprcv);
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  PE::Comm::instance().all_to_all(ptr_tmprcv, nproc, ptr_tmprcv);
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=(double*)PE::Comm::instance().all_to_all((char*)ptr_snddat, nproc, (char*)0, sizeof(double));
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=0.;
  PE::Comm::instance().all_to_all((char*)ptr_snddat, nproc, (char*)ptr_tmprcv, sizeof(double));
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  PE::Comm::instance().all_to_all((char*)ptr_tmprcv, nproc, (char*)ptr_tmprcv, sizeof(double));
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_to_all_vector_constant )
{
  int i;

  setup_data_constant();

  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);
  PE::Comm::instance().all_to_all(vec_snddat, vec_tmprcv);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[i] );
  BOOST_CHECK_EQUAL( (int)vec_tmprcv.size() , rcvcnt );

  vec_tmprcv.assign(nproc*nproc,0.);
  PE::Comm::instance().all_to_all(vec_snddat, vec_tmprcv);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[i] );

  vec_tmprcv=vec_snddat;
  PE::Comm::instance().all_to_all(vec_tmprcv, vec_tmprcv);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[i] );

  vec_tmprcvchr.resize(0);
  vec_tmprcvchr.reserve(0);
  PE::Comm::instance().all_to_all(vec_snddatchr, vec_tmprcvchr );
  BOOST_CHECK_EQUAL( vec_tmprcvchr.size() , sizeof(double)*rcvcnt );
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=0.;
  PE::Comm::instance().all_to_all(vec_snddatchr, vec_tmprcvchr );
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[i] );

  vec_tmprcvchr.assign((char*)(ptr_snddat),(char*)(ptr_snddat+nproc*nproc));
  PE::Comm::instance().all_to_all(vec_tmprcvchr, vec_tmprcvchr );
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[i] );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_to_all_ptr_variable )
{
  int i,j,k;

  setup_data_variable();

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=PE::Comm::instance().all_to_all(ptr_snddat, ptr_sndcnt, (double*)0, ptr_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[k]=0.;
  PE::Comm::instance().all_to_all(ptr_snddat, ptr_sndcnt, ptr_tmprcv, ptr_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  PE::Comm::instance().all_to_all(ptr_tmprcv, ptr_sndcnt, ptr_tmprcv, ptr_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  ptr_tmprcv=PE::Comm::instance().all_to_all(ptr_snddat, ptr_sndcnt, (double*)0, ptr_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[k]=0.;
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all(ptr_snddat, ptr_sndcnt, ptr_tmprcv, ptr_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all(ptr_tmprcv, ptr_sndcnt, ptr_tmprcv, ptr_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=PE::Comm::instance().all_to_all(ptr_snddat, ptr_sndcnt, ptr_sndmap, (double*)0, ptr_rcvcnt, ptr_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[i*nproc+j]=0.;
  PE::Comm::instance().all_to_all(ptr_snddat, ptr_sndcnt, ptr_sndmap, ptr_tmprcv, ptr_rcvcnt, ptr_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  PE::Comm::instance().all_to_all(ptr_tmprcv, ptr_sndcnt, ptr_sndmap, ptr_tmprcv, ptr_rcvcnt, ptr_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=(double*)PE::Comm::instance().all_to_all((char*)ptr_snddat, ptr_sndcnt, (char*)0, ptr_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[k]=0.;
  PE::Comm::instance().all_to_all((char*)ptr_snddat, ptr_sndcnt, (char*)ptr_tmprcv, ptr_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  PE::Comm::instance().all_to_all((char*)ptr_tmprcv, ptr_sndcnt, (char*)ptr_tmprcv, ptr_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  ptr_tmprcv=(double*)PE::Comm::instance().all_to_all((char*)ptr_snddat, ptr_sndcnt, (char*)0, ptr_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[k]=0.;
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all((char*)ptr_snddat, ptr_sndcnt, (char*)ptr_tmprcv, ptr_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  for(i=0; i<nproc; i++) ptr_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all((char*)ptr_tmprcv, ptr_sndcnt, (char*)ptr_tmprcv, ptr_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( ptr_tmpcnt[i] , ptr_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[k] , ptr_rcvdat[k] );

  delete[] ptr_tmprcv;
  ptr_tmprcv=0;
  ptr_tmprcv=(double*)PE::Comm::instance().all_to_all((char*)ptr_snddat, ptr_sndcnt, ptr_sndmap, (char*)0, ptr_rcvcnt, ptr_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) ptr_tmprcv[i*nproc+j]=0.;
  PE::Comm::instance().all_to_all((char*)ptr_snddat, ptr_sndcnt, ptr_sndmap, (char*)ptr_tmprcv, ptr_rcvcnt, ptr_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  delete[] ptr_tmprcv;
  ptr_tmprcv=new double[nproc*nproc];
  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  PE::Comm::instance().all_to_all((char*)ptr_tmprcv, ptr_sndcnt, ptr_sndmap, (char*)ptr_tmprcv, ptr_rcvcnt, ptr_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<ptr_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ptr_tmprcv[i*nproc+j] , ptr_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_to_all_vector_variable )
{
  int i,j,k;

  setup_data_variable();

  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);
  PE::Comm::instance().all_to_all(vec_snddat, vec_sndcnt, vec_tmprcv, vec_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) vec_tmprcv[k]=0.;
  PE::Comm::instance().all_to_all(vec_snddat, vec_sndcnt, vec_tmprcv, vec_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  vec_tmprcv.resize(nproc*nproc);
  vec_tmprcv.reserve(nproc*nproc);
  for (i=0; i<nproc*nproc; i++) vec_tmprcv[i]=vec_snddat[i];
  PE::Comm::instance().all_to_all(vec_tmprcv, vec_sndcnt, vec_tmprcv, vec_rcvcnt);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all(vec_snddat, vec_sndcnt, vec_tmprcv, vec_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) vec_tmprcv[k]=0.;
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all(vec_snddat, vec_sndcnt, vec_tmprcv, vec_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  vec_tmprcv.resize(nproc*nproc);
  vec_tmprcv.reserve(nproc*nproc);
  for (i=0; i<nproc*nproc; i++) vec_tmprcv[i]=vec_snddat[i];
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all(vec_tmprcv, vec_sndcnt, vec_tmprcv, vec_tmpcnt);
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[k] , vec_rcvdat[k] );

  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);
  PE::Comm::instance().all_to_all(vec_snddat, vec_sndcnt, vec_sndmap, vec_tmprcv, vec_rcvcnt, vec_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) vec_tmprcv[i*nproc+j]=0.;
  PE::Comm::instance().all_to_all(vec_snddat, vec_sndcnt, vec_sndmap, vec_tmprcv, vec_rcvcnt, vec_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  vec_tmprcv.resize(nproc*nproc);
  vec_tmprcv.reserve(nproc*nproc);
  for (i=0; i<nproc*nproc; i++) vec_tmprcv[i]=vec_snddat[i];
  PE::Comm::instance().all_to_all(vec_tmprcv, vec_sndcnt, vec_sndmap, vec_tmprcv, vec_rcvcnt, vec_rcvmap);
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( vec_tmprcv[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  vec_tmprcvchr.resize(0);
  vec_tmprcvchr.reserve(0);
  PE::Comm::instance().all_to_all(vec_snddatchr, vec_sndcnt, vec_tmprcvchr, vec_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) ((double*)(&vec_tmprcvchr[0]))[k]=0.;
  PE::Comm::instance().all_to_all(vec_snddatchr, vec_sndcnt, vec_tmprcvchr, vec_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  vec_tmprcvchr.resize(nproc*nproc*sizeof(double));
  vec_tmprcvchr.reserve(nproc*nproc*sizeof(double));
  for (i=0; i<nproc*nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=vec_snddat[i];
  PE::Comm::instance().all_to_all(vec_tmprcvchr, vec_sndcnt, vec_tmprcvchr, vec_rcvcnt, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  vec_tmprcvchr.resize(0);
  vec_tmprcvchr.reserve(0);
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all(vec_snddatchr, vec_sndcnt, vec_tmprcvchr, vec_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) ((double*)(&vec_tmprcvchr[0]))[k]=0.;
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all(vec_snddatchr, vec_sndcnt, vec_tmprcvchr, vec_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  vec_tmprcvchr.resize(nproc*nproc*sizeof(double));
  vec_tmprcvchr.reserve(nproc*nproc*sizeof(double));
  for (i=0; i<nproc*nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=vec_snddat[i];
  for(i=0; i<nproc; i++) vec_tmpcnt[i]=-1;
  PE::Comm::instance().all_to_all(vec_tmprcvchr, vec_sndcnt, vec_tmprcvchr, vec_tmpcnt, sizeof(double));
  for (i=0; i<nproc; i++) BOOST_CHECK_EQUAL( vec_tmpcnt[i] , vec_rcvcnt[i] );
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[k] , vec_rcvdat[k] );

  vec_tmprcvchr.resize(0);
  vec_tmprcvchr.reserve(0);
  PE::Comm::instance().all_to_all(vec_snddatchr, vec_sndcnt, vec_sndmap, vec_tmprcvchr, vec_rcvcnt, vec_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) ((double*)(&vec_tmprcvchr[0]))[i*nproc+j]=0.;
  //for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) vec_tmprcvchr[i*nproc+j]=0.;
  PE::Comm::instance().all_to_all(vec_snddatchr, vec_sndcnt, vec_sndmap, vec_tmprcvchr, vec_rcvcnt, vec_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

  vec_tmprcvchr.resize(nproc*nproc*sizeof(double));
  vec_tmprcvchr.reserve(nproc*nproc*sizeof(double));
  for (i=0; i<nproc*nproc; i++) ((double*)(&vec_tmprcvchr[0]))[i]=vec_snddat[i];
  PE::Comm::instance().all_to_all(vec_tmprcvchr, vec_sndcnt, vec_sndmap, vec_tmprcvchr, vec_rcvcnt, vec_rcvmap, sizeof(double));
  for (i=0, k=0; i<nproc; i++) for (j=0; j<vec_rcvcnt[i]; j++, k++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i*nproc+j] , vec_rcvdat[k] ); // i*nproc+j is not a bug, check reason at init

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
