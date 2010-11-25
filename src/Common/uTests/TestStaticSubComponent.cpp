// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/iterator.hpp>

#include "Common/Log.hpp"
#include "Common/Component.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CRoot.hpp"
#include "Common/CGroup.hpp"
#include "Common/CLink.hpp"
#include "Common/TypeInfo.hpp"
#include "Common/XmlHelpers.hpp"

using namespace std;
using namespace boost;

using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

class Part : public Component
{
public: // typedefs

  typedef boost::shared_ptr< Part > Ptr;
  typedef boost::shared_ptr< Part const > ConstPtr;

public: // functions

  Part ( const std::string& name ) : Component(name)
  {
    BuildComponent<full>().build(this);
  }

  virtual ~Part() {}

  static std::string type_name () { return std::string("Part"); }

  virtual void define_config_properties () {}

private: // helper functions

  virtual void define_signals () {}

}; // HolderT

////////////////////////////////////////////////////////////////////////////////

template < typename SubCompT >
class HolderT : public Component
{
public: // typedefs

  typedef boost::shared_ptr< HolderT<SubCompT> > Ptr;
  typedef boost::shared_ptr< HolderT<SubCompT> const > ConstPtr;

public: // functions

  HolderT ( const std::string& name ) :
      Component(name),
      m_subcomp ( new SubCompT( "subc" ), Deleter<SubCompT>())
  {
    BuildComponent<full>().build(this);

    add_static_component ( m_subcomp );
  }

  virtual ~HolderT() {}

  static std::string type_name () { return std::string("HolderT_") + SubCompT::type_name(); }

  virtual void define_config_properties () {}

private: // helper functions

  virtual void define_signals () {}

private: // data

  Component::Ptr m_subcomp;

}; // HolderT

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( StaticSubComponent_TestSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( add_component )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr hc ( new HolderT<Part> ( "holder_part" ) );

  BOOST_CHECK_EQUAL ( HolderT<Part>::type_name() , "HolderT_Part" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

