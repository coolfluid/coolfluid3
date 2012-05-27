// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for common::EventHandler"

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <iostream>

#include "common/Core.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/ConnectionManager.hpp"
#include "common/EventHandler.hpp"
#include "common/XML/SignalOptions.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::XML;

//------------------------------------------------------------------------------------------
// Auxiliary classes

//------------------------------------------------------------------------------------------

/// This object representes an iterative algorithm that
/// occasionally triggers the event iteration_done and file_written
///
/// @note IterativeAlgo does not know about the classes FileUpload or DisplayIter

struct IterativeAlgo
{
  void run()
  {
    SignalOptions options;

    options.add("iteration", 0u);

    for( Uint iter = 0; iter < 10; ++iter )
    {

      std::cout << " - iter [" << iter << "]" << std::endl;

      // wait a bit

      boost::this_thread::sleep(boost::posix_time::milliseconds(100));

      // alter the iteration on the options

      options["iteration"].change_value( iter );

      // raise the event

      SignalFrame f = options.create_frame();
      Core::instance().event_handler().raise_event( "iteration_done", f );
    }
  }

  void finish()
  {
    // would writes a file here...

    std::cout << " - writting results file " << std::endl;

    // raise the event

    SignalOptions options;

    options.add("file", URI("file:///localhost/path/to/file.txt") );

    SignalFrame f = options.create_frame();
    Core::instance().event_handler().raise_event( "file_written", f );
  }

};

//------------------------------------------------------------------------------------------

/// This object will be listening to an event file_written
/// and will upload the file to a website once is finished writting
///
/// @note FileUpload does not know about the classes IterativeAlgo

struct FileUpload : public ConnectionManager {

  FileUpload() : triggered(0)
  {

    // this is a simple event registration
    // the event function may be private
    Core::instance().event_handler().connect_to_event( "file_written",
                                                       this,
                                                       &FileUpload::on_file_written );
  }

  /// triggered functions from events must receive SignalArgs
  /// in fact they must conform to the signature of a Signal [ void * ( SignalArgs& ) ]
  void on_file_written( SignalArgs& args )
  {
    SignalOptions options( args );

    URI filepath = options.value<URI>("file");

    std::cout << " + EVENT + uploading file [" << filepath.string() << "]" << std::endl;

    ++triggered;
  }

  Uint triggered; ///< for unit test to check how often the event was triggered

};

//------------------------------------------------------------------------------------------

/// This object will be listening to an event iteration_done
/// and will display the iteration when on_iteration_done
///
/// @note DisplayIter does not know about the classes IterativeAlgo

struct DisplayIter : public ConnectionManager {

  DisplayIter() : triggered(0) {}

  /// triggered functions from events must receive SignalArgs
  /// in fact they must conform to the signature of a Signal [ void * ( SignalArgs& ) ]
  void on_iteration_done( SignalArgs& args )
  {
    SignalOptions options( args );

    Uint current_iter = options.value<Uint>("iteration");

    display_iteration( current_iter );

    ++triggered;
  }

  void display_iteration( Uint iter )
  {
    std::cout << " + EVENT + current iteration [" << iter << "]" << std::endl;
  }

  void start_displaying()
  {
    Core::instance().event_handler().connect_to_event( "iteration_done",
                                                       this,
                                                       &DisplayIter::on_iteration_done );
  }

  void stop_displaying()
  {
    connection("iteration_done")->disconnect();
  }

  Uint triggered; ///< for unit test to check how often the event was triggered

};

//------------------------------------------------------------------------------------------
// test fixtures

struct global_fixture
{
  global_fixture()
  {
    Core::instance().initiate(boost::unit_test::framework::master_test_suite().argc,
                              boost::unit_test::framework::master_test_suite().argv);
  }

  ~global_fixture()
  {
    Core::instance().terminate();
  }
};

struct local_fixture
{
  IterativeAlgo algo;
  FileUpload    writer;
  DisplayIter   display;
};

//------------------------------------------------------------------------------------------

BOOST_GLOBAL_FIXTURE( global_fixture )

BOOST_AUTO_TEST_SUITE( event_handler_test_suite )

//------------------------------------------------------------------------------------------

#if 1

BOOST_FIXTURE_TEST_CASE( raise_event, local_fixture )
{
  std::cout << "raise_event" << std::endl;

  // run but dont react on events

  algo.run();

  BOOST_CHECK ( display.triggered == 0 );
  BOOST_CHECK ( writer.triggered  == 0 );

  // finish and write file

  algo.finish();

  BOOST_CHECK ( display.triggered == 0 );
  BOOST_CHECK ( writer.triggered  == 1 );
}

#endif

#if 1

BOOST_FIXTURE_TEST_CASE( connect_to_event, local_fixture )
{
  std::cout << "connect_to_event" << std::endl;

  // run but dont react on events

  algo.run();

  BOOST_CHECK ( display.triggered == 0 );
  BOOST_CHECK ( writer.triggered  == 0 );

  // start displaying on events

  display.start_displaying();

  algo.run();

  BOOST_CHECK ( display.triggered == 10 );
  BOOST_CHECK ( writer.triggered  == 0 );

  // finish and write file

  algo.finish();

  BOOST_CHECK ( display.triggered == 10 );
  BOOST_CHECK ( writer.triggered  == 1 );
}

#endif

#if 1

BOOST_FIXTURE_TEST_CASE( disconnect_from_event, local_fixture )
{
  std::cout << "disconnect_from_event" << std::endl;

  // start displaying on events

  display.start_displaying();

  algo.run();

  BOOST_CHECK ( display.triggered == 10 );
  BOOST_CHECK ( writer.triggered  == 0 );

  // stop displaying

  display.stop_displaying();

  algo.run();

  BOOST_CHECK ( display.triggered == 10 );
  BOOST_CHECK ( writer.triggered  == 0 );
}

#endif

//------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

//------------------------------------------------------------------------------------------
