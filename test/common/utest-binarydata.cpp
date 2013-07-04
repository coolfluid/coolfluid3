// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Component"

#include <iostream>

#include <boost/mpl/if.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include "common/BinaryDataWriter.hpp"
#include "common/Core.hpp"
#include "common/List.hpp"
#include "common/OptionList.hpp"
#include "common/Table.hpp"

#include "common/PE/Comm.hpp"

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
    distribution_type dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
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
    distribution_type dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    const Uint nb_rows = list.size();
    for(Uint i = 0; i != nb_rows; ++i)
    {
      list[i] = dist(gen);
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
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc,boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().is_active(),true);
}

BOOST_AUTO_TEST_CASE( WriteBinaryData )
{
  common::Component& group = *common::Core::instance().root().create_component("WriteGroup", "cf3.common.Group");
  
  common::Table<Uint>& int_table = *group.create_component< common::Table<Uint> >("IntTable");
  int_table.set_row_size(int_table_cols);
  int_table.resize(int_table_size);
  fill_table(int_table);
  
  common::Table<Real>& real_table = *group.create_component< common::Table<Real> >("RealTable");
  real_table.set_row_size(real_table_cols);
  real_table.resize(real_table_size);
  fill_table(real_table);

  common::List<Uint>& int_list = *group.create_component< common::List<Uint> >("IntList");
  int_list.resize(int_list_size);
  fill_list(int_list);

  common::List<Real>& real_list = *group.create_component< common::List<Real> >("RealList");
  real_list.resize(real_list_size);
  fill_list(real_list);
  
  common::BinaryDataWriter& writer = *group.create_component<common::BinaryDataWriter>("Writer");
  writer.options().set("file", common::URI("binary_data.cfbinxml"));
  
  writer.append_data(int_table);
  writer.append_data(real_table);
  writer.append_data(real_list);
  writer.append_data(int_list);

  writer.close();
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
