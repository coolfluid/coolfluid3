#include <iostream>

#include <QtCore>
#include <QApplication>

#include "GUI/Client/MainWindow.hh"

using namespace std;

using namespace CF::GUI::Client;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  
  MainWindow window;
  window.showMaximized();
  
  return app.exec();
}
