#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// IMPORTANT:
// run it both on 1 and 4 cores

#include "Common/MPI/PEInterface.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

struct PE_Init_Fixture
{
  /// common setup for each test case
  PE_Init_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~PE_Init_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

BOOST_FIXTURE_TEST_SUITE( PE_Init_TestSuite, PE_Init_Fixture )

BOOST_AUTO_TEST_CASE( isinit_preinit )
{
  BOOST_CHECK_EQUAL( PEInterface::getInstance().is_init() , false );
}

BOOST_AUTO_TEST_CASE( allrankzero_preinit )
{
  BOOST_CHECK_EQUAL( PEInterface::getInstance().rank() , (Uint)0 );
}

BOOST_AUTO_TEST_CASE( init )
{
  PEInterface::getInstance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PEInterface::getInstance().is_init() , true );
}

BOOST_AUTO_TEST_CASE( rank_and_size )
{
  BOOST_CHECK_LT( (Uint)PEInterface::getInstance().rank() , (Uint)PEInterface::getInstance().size() );
}

BOOST_AUTO_TEST_CASE( collective_op )
{
  Uint rank_based_sum=0,size_based_sum=0;
  std::vector<Uint> ranklist(PEInterface::getInstance().size(),0);
  mpi::all_gather(PEInterface::getInstance(),PEInterface::getInstance().rank(),ranklist);
  for(Uint i=0; i<(Uint)PEInterface::getInstance().size(); i++) {
    rank_based_sum+=ranklist[i];
    size_based_sum+=(Uint)PEInterface::getInstance().size()-(i+1);
  }
  BOOST_CHECK_EQUAL( rank_based_sum , size_based_sum );
}

BOOST_AUTO_TEST_SUITE_END()



