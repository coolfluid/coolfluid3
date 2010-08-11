#include <QString>
#include <QtTest>

#include "GUI/Client/NBrowser.hpp"

#include "GUI/Client/uTests/NBrowserTest.hpp"

using namespace CF::GUI::Client;
using namespace CF::GUI::ClientTest;

void NBrowserTest::test_generateName()
{
  NBrowser b;

  QCOMPARE(b.generateName(), QString("Browser_0"));
  QCOMPARE(b.generateName(), QString("Browser_1"));
  QCOMPARE(b.generateName(), QString("Browser_2"));

  for(int i = 0 ; i < 15 ; i++)
    b.generateName();

  QCOMPARE(b.generateName(), QString("Browser_18"));
}
