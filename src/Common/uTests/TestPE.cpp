#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/MPI/PEInterface.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

struct PE_Fixture
{
  /// common setup for each test case
  PE_Fixture()
  {
    int    argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;
/*
cout << "STARTUP" << flush;

    PEInterface::getInstance().init(argc,argv);

cout << "INIT... " << getpid() << " " << PEInterface::getInstance().is_init() << " " << PEInterface::getInstance().rank() << " " << PEInterface::getInstance().size() << "\n" << flush;
*/
  }

  /// common tear-down for each test case
  ~PE_Fixture()
  {
//    PEInterface::getInstance().finalize();
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

BOOST_FIXTURE_TEST_SUITE( PE_TestSuite, PE_Fixture )

BOOST_AUTO_TEST_CASE( get_rank )
{
//cout << "BLABLA " << PEInterface::getInstance().rank() << "\n" << flush;
//  BOOST_CHECK_EQUAL( PEInterface::getInstance().rank() , (Uint) 0 );
}

BOOST_AUTO_TEST_SUITE_END()


