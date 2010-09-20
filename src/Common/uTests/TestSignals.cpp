// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/CRoot.hpp"
#include "Common/Log.hpp"
#include "Common/XmlHelpers.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////


/// Mini component class for tesng
/// @author Tiago Quintino
class CSmall : public Component {

public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < CSmall,1 > PROVIDER;
  /// pointer to this type
  typedef boost::shared_ptr<CSmall> Ptr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSmall ( const CName& name ) : Component ( name )
  {
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~CSmall() {}

  /// Get the class name
  static std::string type_name () { return "CSmall"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options ) {}

  void trigger_signal_print_message ( Component& receiver )
  {
    boost::shared_ptr<XmlDoc> xmlroot = XmlOps::create_doc();

    XmlNode& docnode = *XmlOps::goto_doc_node(*xmlroot.get());

    XmlNode& signal_frame = *XmlOps::add_signal_frame(docnode, "", "", "", true );

    XmlParams p ( signal_frame );

    p.add_option<int>( "Counter", 10 );

    receiver.call_signal( "print_message", signal_frame );
  }

  void trigger_signal_list_tree ( Component& receiver )
  {
    boost::shared_ptr<XmlDoc> xmlroot = XmlOps::create_doc();

    XmlNode& docnode = *XmlOps::goto_doc_node(*xmlroot.get());

    XmlNode& signal_frame = *XmlOps::add_signal_frame(docnode, "list_tree", full_path(), receiver.full_path(), false);

//    std::vector < std::pair < Signal::id_t, Signal::desc_t > > lists = receiver.list_signals();
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
  void print_message ( XmlNode& xml )
  {
    XmlParams p (xml);

    CFinfo << "Component [" << name() << "] received counter [" << p.get_option<int>("Counter") << "]" << CFendl;
  }

  //@} END SIGNALS

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( CSmall* self )
  {
    self->regist_signal ( "print_message" , "prints" )->connect ( boost::bind ( &CSmall::print_message, self, _1 ) );
  }

}; // CSmall

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

  small_2->trigger_signal_list_tree ( *root.get() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////


