// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Component"

#include <iostream>

#include <boost/mpl/if.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include "common/Action.hpp"
#include "common/Core.hpp"
#include "common/List.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Table.hpp"

#include "common/PE/Comm.hpp"
#include <common/Environment.hpp>

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

struct BinaryDataFixture
{
  BinaryDataFixture() :
    rank(common::PE::Comm::instance().rank()),
    gen(rank),
    int_table_size(10000+1000*rank),
    real_table_size(20000+2000*rank),
    int_list_size(30000-3000*rank),
    real_list_size(40000+4000*rank)
  {
  }

  template<typename T>
  void fill_table(common::Table<T>& table)
  {
    typedef typename boost::mpl::if_< typename boost::is_integral<T>::type, boost::random::uniform_int_distribution<T>, boost::random::uniform_real_distribution<T> >::type distribution_type;
    distribution_type dist(0, 1);
    const Uint nb_rows = table.size();
    const Uint nb_cols = table.row_size();
    for(Uint i = 0; i != nb_rows; ++i)
    {
      for(Uint j = 0; j != nb_cols; ++j)
      {
        table[i][j] = dist(gen);
      }
    }
  }

  template<typename T>
  void fill_list(common::List<T>& list)
  {
    typedef typename boost::mpl::if_< typename boost::is_integral<T>::type, boost::random::uniform_int_distribution<T>, boost::random::uniform_real_distribution<T> >::type distribution_type;
    distribution_type dist(0, 1);
    const Uint nb_rows = list.size();
    for(Uint i = 0; i != nb_rows; ++i)
    {
      list[i] = dist(gen);
    }
  }

  void fill_list(common::List<bool>& list)
  {
    const Uint nb_rows = list.size();
    for(Uint i = 0; i != nb_rows; ++i)
    {
      list[i] = (i % 2 == 0);
    }
  }

  const Uint rank;

  boost::random::mt19937 gen;

  const Uint int_table_size;
  static const Uint int_table_cols = 3;
  const Uint real_table_size;
  static const Uint real_table_cols = 8;

  const Uint int_list_size;
  const Uint real_list_size;
};

BOOST_FIXTURE_TEST_SUITE( BinaryDataSuite, BinaryDataFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::Core::instance().environment().options().set("log_level", 4);
  common::Core::instance().environment().options().set("only_cpu0_writes", false);
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc,boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().is_active(),true);
}

BOOST_AUTO_TEST_CASE( CompareArrays )
{
  common::Component& group = *common::Core::instance().root().create_component("Group", "cf3.common.Group");
  Handle<common::Action> array_differ(group.create_component("ArrayDiff", "cf3.common.ArrayDiff"));
  
  common::Table<Uint>& int_table1 = *group.create_component< common::Table<Uint> >("IntTable1");
  common::Table<Uint>& int_table2 = *group.create_component< common::Table<Uint> >("IntTable2");
  int_table1.set_row_size(int_table_cols);
  int_table1.resize(int_table_size);
  fill_table(int_table1);
  int_table2.set_row_size(int_table_cols);
  int_table2.resize(int_table_size);
  int_table2.array() = int_table1.array();
  array_differ->options().set("left", int_table1.handle());
  array_differ->options().set("right", int_table2.handle());
  array_differ->execute();
  BOOST_CHECK(array_differ->properties().value<bool>("arrays_equal"));
  int_table2[rank][rank] += rank+1;
  array_differ->execute();
  BOOST_CHECK(!array_differ->properties().value<bool>("arrays_equal"));
  
  common::Table<Real>& real_table1 = *group.create_component< common::Table<Real> >("RealTable1");
  common::Table<Real>& real_table2 = *group.create_component< common::Table<Real> >("RealTable2");
  real_table1.set_row_size(real_table_cols);
  real_table1.resize(real_table_size);
  real_table2.set_row_size(real_table_cols);
  real_table2.resize(real_table_size);
  fill_table(real_table1);
  real_table2.array() = real_table1.array();
  array_differ->options().set("left", real_table1.handle());
  array_differ->options().set("right", real_table2.handle());
  array_differ->execute();
  BOOST_CHECK(array_differ->properties().value<bool>("arrays_equal"));
  real_table2[rank][rank] += static_cast<Real>(rank+1)*1e-18;
  array_differ->execute();
  BOOST_CHECK(array_differ->properties().value<bool>("arrays_equal"));
  real_table2[rank][rank] += static_cast<Real>(rank+1)*1e-15;
  array_differ->execute();
  BOOST_CHECK(!array_differ->properties().value<bool>("arrays_equal"));

  common::List<Uint>& int_list1 = *group.create_component< common::List<Uint> >("IntList1");
  common::List<Uint>& int_list2 = *group.create_component< common::List<Uint> >("IntList2");
  int_list1.resize(int_list_size);
  int_list2.resize(int_list_size);
  fill_list( int_list1 );
  int_list2.array() = int_list1.array();
  array_differ->options().set("left", int_list1.handle());
  array_differ->options().set("right", int_list2.handle());
  array_differ->execute();
  BOOST_CHECK(array_differ->properties().value<bool>("arrays_equal"));
  int_list1[rank] += rank+1;
  array_differ->execute();
  BOOST_CHECK(!array_differ->properties().value<bool>("arrays_equal"));

  common::List<Real>& real_list1 = *group.create_component< common::List<Real> >("RealList");
  common::List<Real>& real_list2 = *group.create_component< common::List<Real> >("RealList");
  real_list1.resize(real_list_size);
  real_list2.resize(real_list_size);
  fill_list( real_list1 );
  real_list2.array() = real_list1.array();
  array_differ->options().set("left", real_list1.handle());
  array_differ->options().set("right", real_list2.handle());
  array_differ->execute();
  BOOST_CHECK(array_differ->properties().value<bool>("arrays_equal"));
  real_list2[rank] += static_cast<Real>(rank+1)*1e-18;
  array_differ->execute();
  BOOST_CHECK(array_differ->properties().value<bool>("arrays_equal"));
  real_list2[rank] += static_cast<Real>(rank+1)*1e-14;
  array_differ->execute();
  BOOST_CHECK(!array_differ->properties().value<bool>("arrays_equal"));

  common::List<bool>& bool_list1 = *group.create_component< common::List<bool> >("BoolList1");
  common::List<bool>& bool_list2 = *group.create_component< common::List<bool> >("BoolList2");
  bool_list1.resize(int_list_size);
  bool_list2.resize(int_list_size);
  fill_list( bool_list1 );
  bool_list2.array() = bool_list1.array();
  array_differ->options().set("left", bool_list1.handle());
  array_differ->options().set("right", bool_list2.handle());
  array_differ->execute();
  BOOST_CHECK(array_differ->properties().value<bool>("arrays_equal"));
  bool_list1[rank] = !bool_list2[rank];
  array_differ->execute();
  BOOST_CHECK(!array_differ->properties().value<bool>("arrays_equal"));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
