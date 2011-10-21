// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the options facility"

#include <boost/assign/std/vector.hpp>
#include <boost/mpl/if.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include "common/BasicExceptions.hpp"
#include "common/Group.hpp"
#include "common/Core.hpp"
#include "common/Root.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"

using namespace std;
using namespace boost::assign;

using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( OptionsSuite )

////////////////////////////////////////////////////////////////////////////////

//////////////////////////    Implementation details ///////////////////////////

/// Helper to choose the appropriate return type
template<typename T>
struct SelectOptionType
{
  typedef typename boost::mpl::if_
  <
    boost::is_base_of<Component, T>, // If T is a component...
    OptionComponent<T>,              // we return an OptionComponent
    OptionT<T>                       // otherwise we have a generic option
  >::type type;
};

/// Specialization for URI
template<>
struct SelectOptionType<URI>
{
  typedef OptionURI type;
};

/// Specialization for OptionArray
template<typename T>
struct SelectOptionType< std::vector<T> >
{
  typedef OptionArrayT<T> type;
};

/// Shortcut to choose the appropriate value type
template<typename T>
struct SelectValueType
{
  typedef typename SelectOptionType<T>::type::value_type type;
};

//////////////////////////        Proposed API       ///////////////////////////

/// Add option, variant with default parameter
template<typename T>
typename SelectOptionType<T>::type& add_option(OptionList& options, const std::string& name, const typename SelectValueType<T>::type& default_value = typename SelectValueType<T>::type())
{
  typedef typename SelectOptionType<T>::type OptionType;
  return dynamic_cast<OptionType&>(*options.add_option<OptionType>(name, default_value));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( StringOption )
{
  Root& root = Core::instance().root();

  //root.options().add_option< OptionT<std::string> >("test_option", "Test Option", "test01");
  root.options().add_option< OptionT<std::string> >( "test_option", "test01");
  BOOST_CHECK_EQUAL(root.option("test_option").value_str(), "test01");

  root.option("test_option").change_value(std::string("test02"));
  BOOST_CHECK_EQUAL(root.option("test_option").value_str(), "test02");
}

BOOST_AUTO_TEST_CASE( ComponentOption )
{
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionAborts = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  Root& root = Core::instance().root();
  const Component& referred = root.create_component<Component>("ReferredComponent");

  //OptionComponent<Component>::Ptr opt = boost::dynamic_pointer_cast< OptionComponent<Component> >(root.options().add_option< OptionComponent<Component> >("test_component_option", "Test component option", root.uri()));
  OptionComponent<Component>& opt = add_option<Component>(root.options(), "test_component_option", root.uri());
  BOOST_CHECK(root.uri() == root.option("test_component_option").value<URI>());
  BOOST_CHECK(root.name() == opt.component().name());

  root.option("test_component_option").change_value(referred.uri());
  BOOST_CHECK(referred.uri() == root.option("test_component_option").value<URI>());
  BOOST_CHECK(referred.name() == opt.component().name());

  const Group& group = root.create_component<Group>("TestGroup");
  //OptionComponent<Group>::Ptr group_opt = boost::dynamic_pointer_cast< OptionComponent<Group> >(root.options().add_option< OptionComponent<Group> >("test_group_option", "Test group option", URI()));
  OptionComponent<Group>& group_opt = add_option<Group>(root.options(), "test_group_option");
  try
  {
    root.option("test_group_option").change_value(referred.uri());
    BOOST_CHECK(false); // Above should throw
  }
  catch(CastingFailed&)
  {
    BOOST_CHECK(true);
  }

  root.option("test_group_option").change_value(group.uri());
  BOOST_CHECK(&group == &group_opt.component());
}

BOOST_AUTO_TEST_CASE( TestOptionArray )
{
  Root& root = Core::instance().root();

  std::vector<int> def;
  def += 1,2,3,4,5,6,7,8,9;
  BOOST_CHECK(add_option< std::vector<int> >(root.options(), "test_array_option", def).value_vect() == def);

  BOOST_CHECK(def == root.option("test_array_option").value< std::vector<int> >());
}

BOOST_AUTO_TEST_CASE( TestOptionURI )
{
  Root& root = Core::instance().root();

  // Since the result is properly typed, we can immediately call supported_protocol
  add_option<URI>(root.options(), "test_uri_option", root.uri()).supported_protocol(cf3::common::URI::Scheme::CPATH);

  BOOST_CHECK(root.uri() == root.option("test_uri_option").value< URI >());
}


//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
