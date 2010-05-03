#include <iostream>

#include <QtCore>
#include <QApplication>

#include "Common/CF.hpp"
#include "Common/Exception.hpp"

#include "GUI/Client/MainWindow.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  int returnValue;
  CF::AssertionManager::getInstance().AssertionThrows = true;

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

  return returnValue ;
}
