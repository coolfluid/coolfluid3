// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF signals"

#include <boost/test/unit_test.hpp>

#include "Common/Signal.hpp"
#include "Common/CRoot.hpp"
#include "Common/LibCommon.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"
#include "Common/OptionT.hpp"

#include "Common/XML/SignalOptions.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////


/// Mini component class for tesng
/// @author Tiago Quintino
class CSmall : public Component {

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CSmall> Ptr;
  typedef boost::shared_ptr<CSmall const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSmall ( const std::string& name ) : Component ( name )
  {
    this->regist_signal ( "print_message" , "prints" )->signal->connect ( boost::bind ( &CSmall::print_message, this, _1 ) );
  }

  /// Virtual destructor
  virtual ~CSmall() {}

  /// Get the class name
  static std::string type_name () { return "CSmall"; }

  void trigger_signal_print_message ( Component& receiver )
  {
    SignalFrame signal_frame;

    SignalOptions options;

    options.add_option< OptionT<int> >( "Counter", 10 );

    signal_frame = options.create_frame("Target", "//Root", "//Root");

    receiver.call_signal( "print_message", signal_frame );
  }

  void trigger_signal_list_tree( Component& receiver )
  {
    SignalFrame signal_frame( "list_tree", uri(), receiver.uri());


//    std::vector < std::pair < Signal::id_t, SignalName > > lists = receiver.list_signals();
//    for ( int i = 0; i < lists.size(); i++)
//    {
//      CFinfo << "signal [" << lists[i].first << "] desc [" << lists[i].second << "]" << CFendl;
//    }

    receiver.call_signal( "list_tree", signal_frame );

//    XmlOps::print_xml_node( *signal_frame.document() );
  }

  /// @name SIGNALS
  //@{

  /// creates a component from this component
  void print_message ( SignalArgs& xml )
  {
//    XmlParams p (xml);

  //  CFinfo << "Component [" << name() << "] received counter [" << p.get_option<int>("Counter") << "]" << CFendl;
  }

  //@} END SIGNALS

}; // CSmall

Common::ComponentBuilder < CSmall, Component, LibCommon > CSmall_Builder;

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
  CRoot::Ptr root = CRoot::create ( "root" );

  CSmall::Ptr small_1  ( new CSmall ( "small-1" ) );
  CSmall::Ptr small_2  ( new CSmall ( "small-2" ) );

  root->add_component(small_1);
  small_1->add_component(small_2);

  small_1->trigger_signal_print_message ( *small_2.get() );

  small_2->trigger_signal_list_tree( *root.get() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
