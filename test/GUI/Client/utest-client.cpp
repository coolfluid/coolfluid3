// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QCoreApplication>
#include <QtTest>

#include "test/GUI/Client/CNodeTest.hpp"
#include "test/GUI/Client/CommonFunctions.hpp"
#include "test/GUI/Client/NBrowserTest.hpp"
#include "test/GUI/Client/NTreeTest.hpp"
#include "test/GUI/Client/NLinkTest.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientTest;

int main(int argc, char * argv[])
{
  QCoreApplication app(argc, argv);
  int passed = 0;

  CF::AssertionManager::instance().AssertionThrows = true;

  // CommonFunctionsTest must be the first to be run !!
  passed += QTest::qExec(new CommonFunctionsTest, argc, argv);
  passed += QTest::qExec(new NTreeTest(), argc, argv);
//  passed += QTest::qExec(new CNodeTest(), argc, argv);
//  passed += QTest::qExec(new NBrowserTest(), argc, argv);
//  passed += QTest::qExec(new NLinkTest(), argc, argv);

  return passed;
}
