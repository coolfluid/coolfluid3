#include <QtCore>
#include <QtTest>

#include "GUI/Client/uTests/CNodeTest.hpp"
#include "GUI/Client/uTests/NTreeTest.hpp"

using namespace CF::GUI::ClientTest;

int main(int argc, char * argv[])
{
  QApplication app(argc, argv);
  int passed = 0;

  passed += QTest::qExec(new NTreeTest(), argc, argv);
  passed += QTest::qExec(new CNodeTest(), argc, argv);

  return passed;
}
