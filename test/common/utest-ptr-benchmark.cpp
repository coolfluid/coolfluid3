// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Map component"

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "common/CF.hpp"
#include "common/Handle.hpp"

#include "Tools/Testing/TimedTestFixture.hpp"



//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

struct PtrFixture : Tools::Testing::TimedTestFixture
{
  typedef boost::shared_ptr<Uint> SharedT;
  typedef boost::weak_ptr<Uint> WeakT;
  typedef Handle<Uint> HandleT;
  typedef Uint* RawT;

  SharedT* shared_vec()
  {
    static SharedT* v = 0;
    if(!v)
      v = new SharedT[vec_size];
    return v;
  }

  WeakT* weak_vec()
  {
    static WeakT* v = 0;
    if(!v)
      v = new WeakT[vec_size];
    return v;
  }

  HandleT* handle_vec()
  {
    static HandleT* v = 0;
    if(!v)
      v = new HandleT[vec_size];
    return v;
  }

  RawT* raw_vec()
  {
    static RawT* v = 0;
    if(!v)
      v = new RawT[vec_size*2];
    return v;
  }

  static const Uint vec_size = 1000000;
  static const Uint deref_repeats = 100;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BuildOptions, PtrFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( FillShared )
{
  std::cout << "size of boost::shared_ptr: " << sizeof(SharedT) << std::endl;
  std::cout << "size of boost::weak_ptr: " << sizeof(WeakT) << std::endl;
  std::cout << "size of raw pointer: " << sizeof(RawT) << std::endl;

  SharedT* vec = shared_vec();
  for(Uint i = 0; i != vec_size; ++i)
    vec[i] = boost::shared_ptr<Uint>(new Uint(i));
}

BOOST_AUTO_TEST_CASE ( FillWeak )
{
  SharedT* s_vec = shared_vec();
  WeakT* w_vec = weak_vec();
  for(Uint i = 0; i != vec_size; ++i)
    w_vec[i] = s_vec[i];
}

BOOST_AUTO_TEST_CASE ( FillHandle )
{
  SharedT* s_vec = shared_vec();
  HandleT* w_vec = handle_vec();
  for(Uint i = 0; i != vec_size; ++i)
    w_vec[i] = HandleT(s_vec[i]);
}

BOOST_AUTO_TEST_CASE ( FillRaw )
{
  RawT* vec = raw_vec();
  for(Uint i = 0; i != 2*vec_size; ++i)
    vec[i] = new Uint(i/2);
}

BOOST_AUTO_TEST_CASE ( CopyShared )
{
  SharedT* new_vec = new SharedT[vec_size];
  SharedT* old_vec = shared_vec();
  for(Uint i = 0; i != vec_size; ++i)
    new_vec[i] = old_vec[i];
}

BOOST_AUTO_TEST_CASE ( CopyWeak )
{
  WeakT* new_vec = new WeakT[vec_size];
  WeakT* old_vec = weak_vec();
  for(Uint i = 0; i != vec_size; ++i)
    new_vec[i] = old_vec[i];
}

BOOST_AUTO_TEST_CASE ( CopyHandle )
{
  HandleT* new_vec = new HandleT[vec_size];
  HandleT* old_vec = handle_vec();
  for(Uint i = 0; i != vec_size; ++i)
    new_vec[i] = old_vec[i];
}

BOOST_AUTO_TEST_CASE ( CopyRaw )
{
  RawT* new_vec = new RawT[vec_size*2];
  RawT* old_vec = raw_vec();
  for(Uint i = 0; i != vec_size; ++i)
    new_vec[2*i] = old_vec[2*i];
}

BOOST_AUTO_TEST_CASE ( CheckShared )
{
  SharedT* vec = shared_vec();
  Uint result = 0;
  for(Uint i = 0; i != vec_size; ++i)
    result += is_not_null(vec[i].get());
  BOOST_CHECK(vec_size == result);
}

BOOST_AUTO_TEST_CASE ( CheckWeak)
{
  WeakT* vec = weak_vec();
  Uint result = 0;
  for(Uint i = 0; i != vec_size; ++i)
    result += !vec[i].expired();
  BOOST_CHECK(vec_size == result);
}

BOOST_AUTO_TEST_CASE ( CheckHandle )
{
  HandleT* vec = handle_vec();
  Uint result = 0;
  for(Uint i = 0; i != vec_size; ++i)
    result += is_not_null(vec[i]);
  BOOST_CHECK(vec_size == result);
}

BOOST_AUTO_TEST_CASE ( CheckRaw )
{
  RawT* vec = raw_vec();
  Uint result = 0;
  for(Uint i = 0; i != vec_size; ++i)
    result += is_not_null(vec[2*i]);
  BOOST_CHECK(vec_size == result);
}

BOOST_AUTO_TEST_CASE ( DerefShared )
{
  SharedT* vec = shared_vec();
  Uint result = 0;
  for(Uint r = 0; r != deref_repeats; ++r)
  {
    for(Uint i = 0; i != vec_size; ++i)
    {
      result += *vec[i];
    }
  }
  BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE ( DerefWeak)
{
  WeakT* vec = weak_vec();
  Uint result = 0;
  for(Uint r = 0; r != deref_repeats; ++r)
  {
    for(Uint i = 0; i != vec_size; ++i)
    {
      result += *vec[i].lock();
    }
  }
  BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE ( DerefHandle )
{
  HandleT* vec = handle_vec();
  Uint result = 0;
  for(Uint r = 0; r != deref_repeats; ++r)
  {
    for(Uint i = 0; i != vec_size; ++i)
    {
      result += *vec[i];
    }
  }
  BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE ( DerefRaw )
{
  RawT* vec = raw_vec();
  Uint result = 0;
  for(Uint r = 0; r != deref_repeats; ++r)
  {
    for(Uint i = 0; i != vec_size; ++i)
    {
      result += *vec[2*i];
    }
  }
  BOOST_CHECK(result);
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

