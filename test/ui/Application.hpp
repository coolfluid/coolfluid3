#ifndef cf3_test_ui_Application_hpp
#define cf3_test_ui_Application_hpp

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <QApplication>

QApplication * application()
{

#ifdef Q_WS_X11
  if( getenv("DISPLAY") == 0 ) // if no graphical environment is found, we exit
  {
    std::cout << "No graphical environment found, exiting..." << std::endl;
    exit(0);
  }
#endif
  static QApplication * app = new QApplication(
        boost::unit_test::framework::master_test_suite().argc,
        boost::unit_test::framework::master_test_suite().argv
        );

  return app;
}

#endif // cf3_test_ui_Application_hpp
