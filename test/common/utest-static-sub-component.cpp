// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for static sub-components"

#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/iterator.hpp>

#include "common/Log.hpp"
#include "common/Component.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"
#include "common/Link.hpp"

using namespace std;
using namespace boost;

using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

class Part : public Component
{

public: // functions

  Part ( const std::string& name ) : Component(name)
  {
  }

  virtual ~Part() {}

  static std::string type_name () { return std::string("Part"); }

}; // HolderT

////////////////////////////////////////////////////////////////////////////////

template < typename SubCompT >
class HolderT : public Component
{
public: // functions

  HolderT ( const std::string& name ) :
      Component(name),
      m_subcomp (create_static_component<SubCompT>("subc"))
  {
  }

  virtual ~HolderT() {}

  static std::string type_name () { return std::string("HolderT_") + SubCompT::type_name(); }

private: // data

  Handle<SubCompT> m_subcomp;

}; // HolderT

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( StaticSubComponent_TestSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( add_component )
{
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  Handle<Component> cp = root->create_component< HolderT<Part> >( "myHolderT_Part" );

  BOOST_CHECK_EQUAL ( HolderT<Part>::type_name() , "HolderT_Part" );

  /// @todo makes the utest crash (bat_weak_ptr)
//  BOOST_CHECK_NO_THROW ( cp->get_child_ptr("subc").get() );

  BOOST_CHECK_EQUAL ( cp->name(), std::string("myHolderT_Part"));

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

