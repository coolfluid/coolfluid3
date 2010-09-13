#include <iostream>

#include <QtCore>
#include <QApplication>

#include "Common/CF.hpp"
#include "Common/Exception.hpp"

#include "Common/CRoot.hpp"
#include "Common/CLink.hpp"

#include "GUI/Client/MainWindow.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  int returnValue;
  CF::AssertionManager::instance().AssertionThrows = true;
  CF::AssertionManager::instance().AssertionDumps = true;

  try
  {
   MainWindow window;
   window.showMaximized();
   returnValue = app.exec();
  }
  catch(Exception e)
  {
    std::cerr << "Application stopped on uncaught exception:" << std::endl;
    std::cerr << e.what() << std::endl;
    returnValue = -1;
  }

  return returnValue;
}
