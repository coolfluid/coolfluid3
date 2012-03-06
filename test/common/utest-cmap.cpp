// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Map component"

#include <boost/test/unit_test.hpp>

#include "common/CF.hpp"
#include "common/Map.hpp"
#include "common/Log.hpp"
#include "common/Foreach.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;

struct MapFixture
{
  /// common setup for each test case
  MapFixture()
  {    
  }

  /// common tear-down for each test case
  ~MapFixture()
  {
  }

private:

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MapTests, MapFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_Map )
{

  boost::shared_ptr< Map<std::string,Uint> > map_ptr ( allocate_component< Map<std::string,Uint> > ("map"));
  Map<std::string,Uint>& map = *map_ptr;
  
  
  BOOST_CHECK_EQUAL(map.type_name() , "Map<string,unsigned>");
  
  BOOST_CHECK(map.find("first") == map.end());
  
  map.push_back(std::string("first"), (Uint) 1);
  BOOST_CHECK_EQUAL(map.size() , 1u);
  BOOST_CHECK_EQUAL(map["first"] , 1u);
  map.push_back(std::string("second"), (Uint) 2);
  BOOST_CHECK_EQUAL(map.size() , 2u);
  BOOST_CHECK_EQUAL(map["second"] , 2u);
  map["third"] = 3;
  BOOST_CHECK_EQUAL(map.size() , 3u);
  BOOST_CHECK_EQUAL(map["third"] , 3u);
  
  BOOST_CHECK(map.find("fourth") == map.end());
  map["fourth"] = 4;
  BOOST_CHECK_EQUAL(map.find("fourth")->second,4u);
  BOOST_CHECK_EQUAL(map.size() , 4u);
  

}

BOOST_AUTO_TEST_CASE ( test_Map_looping )
{

  boost::shared_ptr< Map<std::string,Uint> > map_ptr ( allocate_component< Map<std::string,Uint> > ("map"));
  Map<std::string,Uint>& map = *map_ptr;
  
  
  map.push_back(std::string("first"), (Uint) 1);
  map.push_back(std::string("second"), (Uint) 2);
  map.push_back(std::string("third"), (Uint) 3);
  map.push_back(std::string("fourth"), (Uint) 4);
  
  map.sort_keys();
  
  foreach_container((const std::string& key)(Uint data),map)
    BOOST_CHECK_EQUAL( map[key] , data );
  
  std::pair<Map<std::string,Uint>::iterator,bool> ret;
  ret = map.insert(std::make_pair("fifth",5u));
  BOOST_CHECK_EQUAL(ret.second, true);  // --> new element
  BOOST_CHECK_EQUAL(ret.first->first, "fifth");
  BOOST_CHECK_EQUAL(ret.first->second, 5u);
  
  // try to insert one that is already there
  ret = map.insert(std::make_pair("fifth",1000));
  BOOST_CHECK_EQUAL(ret.second, false); // --> already exists
  BOOST_CHECK_EQUAL(ret.first->first, "fifth");
  BOOST_CHECK_EQUAL(ret.first->second, 5u);  // --> value did not change
  
  BOOST_CHECK(map.erase("first"));
  BOOST_CHECK(map.find("first") == map.end());
  
  map.erase( map.find("fourth") );
  BOOST_CHECK(map.find("fourth") == map.end());
  
  const Map<std::string,Uint>& const_map = *map_ptr;
  BOOST_CHECK(const_map.find("second") != const_map.end());
  
}

BOOST_AUTO_TEST_CASE ( test_Map_exceptions )
{

  boost::shared_ptr< Map<int, int> > map_ptr ( allocate_component< Map<int,int> > ("map"));
  Map<int,int>& map = *map_ptr;
  
  BOOST_CHECK_EQUAL(map.type_name() , "Map<integer,integer>");
  
  map.push_back(1,1);
  map.push_back(2,2);
  map.push_back(3,3);
  map.push_back(4,4);
  
  // const Map<int,int>& const_map = *map_ptr;
  // BOOST_CHECK_THROW(const_map.find(2),IllegalCall);
  // BOOST_CHECK_THROW(const_map[2],IllegalCall);
  
  map.sort_keys();
  // BOOST_CHECK_THROW(const_map[6],ValueNotFound);
    
  map.push_back(2,3);  // adding another entry with key 2 should not be allowed
  // BOOST_CHECK_THROW(map.sort_keys(),ValueExists);
    
}

BOOST_AUTO_TEST_CASE ( test_Map_copy_std_map )
{

  boost::shared_ptr< Map<std::string,int> > map_ptr ( allocate_component< Map<std::string,int> > ("map"));
  Map<std::string,int>& map = *map_ptr;
  
  BOOST_CHECK_EQUAL(map.type_name() , "Map<string,integer>");

  
  std::map<std::string,int> stl_map;
  stl_map["first"] = 1;
  stl_map["second"] = 2;
  stl_map["third"] = 3;
  stl_map["fourth"] = 4;
  
  map.copy_std_map(stl_map);
  BOOST_CHECK_EQUAL(map.size(), 4u);
  BOOST_CHECK_EQUAL(map["third"], 3);
}

//////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

