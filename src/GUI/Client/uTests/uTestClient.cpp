#include <QApplication>
#include <QtTest>

#include "GUI/Client/uTests/CNodeTest.hpp"
#include "GUI/Client/uTests/NBrowserTest.hpp"
#include "GUI/Client/uTests/NTreeTest.hpp"
#include "GUI/Client/uTests/NLinkTest.hpp"

using namespace CF::GUI::ClientTest;

int main(int argc, char * argv[])
{
  QApplication app(argc, argv);
  int passed = 0;

  passed += QTest::qExec(new NTreeTest(), argc, argv);
  passed += QTest::qExec(new CNodeTest(), argc, argv);
  passed += QTest::qExec(new NBrowserTest(), argc, argv);
  passed += QTest::qExec(new NLinkTest(), argc, argv);

  return passed;
}
