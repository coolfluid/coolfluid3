#include <QtCore>
#include <QtTest>

#include "GUI/Client/uTests/TestNTree.hpp"

using namespace CF::GUI::ClientTest;

int main(int argc, char * argv[])
{
  QApplication app(argc, argv);
  bool passed = true;

  passed &= QTest::qExec(new NTreeTest(), argc, argv);

  return !passed;
}
