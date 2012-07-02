// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF signals"

#include <boost/test/unit_test.hpp>

#include "common/Signal.hpp"
#include "common/LibCommon.hpp"
#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/Group.hpp"

#include "common/XML/SignalOptions.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////////


/// Mini component class for tesng
/// @author Tiago Quintino
class CSmall : public Component
{
public: // functions

  /// Contructor
  /// @param name of the component
  CSmall ( const std::string& name ) : Component ( name )
  {
    this->regist_signal( "print_message" )
    .description("prints")
    .pretty_name("").connect ( boost::bind ( &CSmall::print_message, this, _1 ) );
  }

  /// Virtual destructor
  virtual ~CSmall() {}

  /// Get the class name
  static std::string type_name () { return "CSmall"; }

  void trigger_signal_print_message ( Component& receiver )
  {
    SignalFrame signal_frame;

    SignalOptions options;

    options.add( "Counter", 10 );

    signal_frame = options.create_frame("Target", "/", "/");

    receiver.call_signal( "print_message", signal_frame );
  }

  void trigger_signal_list_tree( Component& receiver )
  {
    SignalFrame signal_frame( "list_tree", uri(), receiver.uri());

    receiver.call_signal( "list_tree", signal_frame );

//    XmlOps::print_xml_node( *signal_frame.document() );
  }

  /// @name SIGNALS
  //@{

  /// creates a component from this component
  void print_message ( SignalArgs& args )
  {
//    XmlParams p (xml);

  //  CFinfo << "Component [" << name() << "] received counter [" << p.get_option<int>("Counter") << "]" << CFendl;
  }

  //@} END SIGNALS

}; // CSmall

common::ComponentBuilder < CSmall, Component, LibCommon > CSmall_Builder;

////////////////////////////////////////////////////////////////////////////////

struct TestSignals_Fixture
{
  /// common setup for each test case
  TestSignals_Fixture()  {}

  /// common tear-down for each test case
  ~TestSignals_Fixture() {}

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestSignals_TestSuite, TestSignals_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( simple_signal )
{
  boost::shared_ptr<Group> root = allocate_component<Group>("root");

  boost::shared_ptr<CSmall> small_1  ( allocate_component<CSmall> ( "small-1" ) );
  boost::shared_ptr<CSmall> small_2  ( allocate_component<CSmall> ( "small-2" ) );

  root->add_component(small_1);
  small_1->add_component(small_2);

  small_1->trigger_signal_print_message ( *small_2.get() );

  small_2->trigger_signal_list_tree( *root.get() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
