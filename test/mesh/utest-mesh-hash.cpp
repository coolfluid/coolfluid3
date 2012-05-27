// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh interpolation"

#include <boost/test/unit_test.hpp>
#include "common/Log.hpp"
#include "common/OptionList.hpp"


#include "mesh/ParallelDistribution.hpp"
#include "mesh/MergedParallelDistribution.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;

////////////////////////////////////////////////////////////////////////////////

struct TestParallelDistribution_Fixture
{
  /// common setup for each test case
  TestParallelDistribution_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestParallelDistribution_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestParallelDistribution_TestSuite, TestParallelDistribution_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors)
{
  boost::shared_ptr<ParallelDistribution> hash = allocate_component<ParallelDistribution>("hash");
  BOOST_CHECK_EQUAL(hash->name(),"hash");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( SingleHash )
{
  boost::shared_ptr<ParallelDistribution> hash = allocate_component<ParallelDistribution>("hash");
  hash->options().set("nb_obj", (Uint) 11);
  hash->options().set("nb_parts", (Uint) 3);

  BOOST_CHECK(true);

  BOOST_CHECK_EQUAL(hash->part_of_obj(0), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->part_of_obj(2), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->part_of_obj(3), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->part_of_obj(5), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->part_of_obj(6), (Uint) 2);
  BOOST_CHECK_EQUAL(hash->part_of_obj(10), (Uint) 2);

  BOOST_CHECK_EQUAL(hash->start_idx_in_part(0), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->start_idx_in_part(1), (Uint) 3);
  BOOST_CHECK_EQUAL(hash->start_idx_in_part(2), (Uint) 6);

  BOOST_CHECK_EQUAL(hash->end_idx_in_part(0), (Uint) 3);
  BOOST_CHECK_EQUAL(hash->end_idx_in_part(1), (Uint) 6);
  BOOST_CHECK_EQUAL(hash->end_idx_in_part(2), (Uint) 11);

  BOOST_CHECK_EQUAL(hash->nb_objects_in_part(0), (Uint) 3);
  BOOST_CHECK_EQUAL(hash->nb_objects_in_part(1), (Uint) 3);
  BOOST_CHECK_EQUAL(hash->nb_objects_in_part(2), (Uint) 5);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( MixedHash )
{
  boost::shared_ptr<MergedParallelDistribution> hash = allocate_component<MergedParallelDistribution>("hash");
  std::vector<Uint> num_obj(2);
  num_obj[0] = 10;
  num_obj[1] = 8;
  hash->options().set("nb_parts", (Uint) 3);
  hash->options().set("nb_obj", num_obj);

  BOOST_CHECK(true);

  BOOST_CHECK_EQUAL(hash->subhash(0).part_of_obj(0), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->subhash(0).part_of_obj(3), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->subhash(0).part_of_obj(6), (Uint) 2);
  BOOST_CHECK_EQUAL(hash->subhash(1).part_of_obj(0), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->subhash(1).part_of_obj(2), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->subhash(1).part_of_obj(4), (Uint) 2);

  BOOST_CHECK_EQUAL(hash->subhash(0).part_size(), (Uint) 3);
  BOOST_CHECK_EQUAL(hash->subhash(1).part_size(), (Uint) 2);
  BOOST_CHECK_EQUAL(hash->part_size(), (Uint) 5);

  BOOST_CHECK_EQUAL(hash->part_of_obj(0), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->part_of_obj(2), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->part_of_obj(3), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->part_of_obj(4), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->part_of_obj(5), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->part_of_obj(7), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->part_of_obj(8), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->part_of_obj(9), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->part_of_obj(10), (Uint) 2);
  BOOST_CHECK_EQUAL(hash->part_of_obj(13), (Uint) 2);
  BOOST_CHECK_EQUAL(hash->part_of_obj(14), (Uint) 2);
  BOOST_CHECK_EQUAL(hash->part_of_obj(16), (Uint) 2);

  BOOST_CHECK_EQUAL(hash->start_idx_in_part(0), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->start_idx_in_part(1), (Uint) 5);
  BOOST_CHECK_EQUAL(hash->start_idx_in_part(2), (Uint) 10);

  BOOST_CHECK_EQUAL(hash->end_idx_in_part(0), (Uint) 5);
  BOOST_CHECK_EQUAL(hash->end_idx_in_part(1), (Uint) 10);
  BOOST_CHECK_EQUAL(hash->end_idx_in_part(2), (Uint) 18);

  BOOST_CHECK_EQUAL(hash->nb_objects_in_part(0), (Uint) 5);
  BOOST_CHECK_EQUAL(hash->nb_objects_in_part(1), (Uint) 5);
  BOOST_CHECK_EQUAL(hash->nb_objects_in_part(2), (Uint) 8);


  BOOST_CHECK_EQUAL(hash->subhash_of_obj(0), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(2), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(3), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(4), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(5), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(7), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(8), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(9), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(10), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(13), (Uint) 0);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(14), (Uint) 1);
  BOOST_CHECK_EQUAL(hash->subhash_of_obj(16), (Uint) 1);


}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

