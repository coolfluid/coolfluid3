#ifndef cf3_test_ui_CoreApplication_hpp
#define cf3_test_ui_CoreApplication_hpp

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include <QCoreApplication>

QCoreApplication & application()
{
  static QCoreApplication app( boost::unit_test::framework::master_test_suite().argc,
                           boost::unit_test::framework::master_test_suite().argv );
  return app;
}

#endif // cf3_test_ui_CoreApplication_hpp
