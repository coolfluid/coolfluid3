// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Map component"

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "common/CF.hpp"

#include "Tools/Testing/TimedTestFixture.hpp"



//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

struct PtrFixture : Tools::Testing::TimedTestFixture
{
  typedef std::vector< boost::shared_ptr<Uint> > SharedVecT;
  typedef std::vector< boost::weak_ptr<Uint> > WeakVecT;
  typedef std::vector< Uint* > RawVecT;

  SharedVecT& shared_vec()
  {
    static SharedVecT v;
    return v;
  }

  WeakVecT& weak_vec()
  {
    static WeakVecT v;
    return v;
  }

  RawVecT& raw_vec()
  {
    static RawVecT v;
    return v;
  }

  static const Uint vec_size = 1000000;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BuildOptions, PtrFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( Initialize )
{
  shared_vec().reserve(vec_size);
  weak_vec().reserve(vec_size);
  raw_vec().reserve(vec_size);

  BOOST_CHECK(shared_vec().capacity() == vec_size);
  BOOST_CHECK(weak_vec().capacity() == vec_size);
  BOOST_CHECK(raw_vec().capacity() == vec_size);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( FillShared )
{
  SharedVecT& vec = shared_vec();
  for(Uint i = 0; i != vec_size; ++i)
    vec.push_back(boost::shared_ptr<Uint>(new Uint(i)));
}

BOOST_AUTO_TEST_CASE ( FillWeak )
{
  SharedVecT& s_vec = shared_vec();
  WeakVecT& w_vec = weak_vec();
  for(Uint i = 0; i != vec_size; ++i)
    w_vec.push_back(s_vec[i]);
}

BOOST_AUTO_TEST_CASE ( FillRaw )
{
  RawVecT& vec = raw_vec();
  for(Uint i = 0; i != vec_size; ++i)
    vec.push_back(new Uint(i));
}

BOOST_AUTO_TEST_CASE ( CopyShared )
{
  SharedVecT vec = shared_vec();
  BOOST_CHECK(vec_size == vec.size());
}

BOOST_AUTO_TEST_CASE ( CopyWeak )
{
  WeakVecT vec = weak_vec();
  BOOST_CHECK(vec_size == vec.size());
}

BOOST_AUTO_TEST_CASE ( CopyRaw )
{
  RawVecT vec = raw_vec();
  BOOST_CHECK(vec_size == vec.size());
}

BOOST_AUTO_TEST_CASE ( CheckShared )
{
  SharedVecT& vec = shared_vec();
  for(Uint i = 0; i != vec_size; ++i)
    BOOST_CHECK(is_not_null(vec[i]));
}

BOOST_AUTO_TEST_CASE ( CheckWeak)
{
  WeakVecT& vec = weak_vec();
  for(Uint i = 0; i != vec_size; ++i)
    BOOST_CHECK(!vec[i].expired());
}

BOOST_AUTO_TEST_CASE ( CheckRaw )
{
  RawVecT& vec = raw_vec();
  for(Uint i = 0; i != vec_size; ++i)
    BOOST_CHECK(is_not_null(vec[i]));
}

BOOST_AUTO_TEST_CASE ( DerefShared )
{
  SharedVecT& vec = shared_vec();
  Uint result = 0;
  for(Uint i = 0; i != vec_size; ++i)
  {
    result += *vec[i];
  }
  BOOST_CHECK_EQUAL(((vec_size-1)*vec_size)/2, result);
}

BOOST_AUTO_TEST_CASE ( DerefWeak)
{
  WeakVecT& vec = weak_vec();
  Uint result = 0;
  for(Uint i = 0; i != vec_size; ++i)
  {
    result += *vec[i].lock();
  }
  BOOST_CHECK_EQUAL(((vec_size-1)*vec_size)/2, result);
}

BOOST_AUTO_TEST_CASE ( DerefRaw )
{
  RawVecT& vec = raw_vec();
  Uint result = 0;
  for(Uint i = 0; i != vec_size; ++i)
  {
    result += *vec[i];
  }
  BOOST_CHECK_EQUAL(((vec_size-1)*vec_size)/2, result);
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

