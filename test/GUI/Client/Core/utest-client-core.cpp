// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QCoreApplication>
#include <QtTest>

#include "test/GUI/Client/Core/CommitDetailsTest.hpp"
#include "test/GUI/Client/Core/CommonFunctions.hpp"
#include "test/GUI/Client/Core/CNodeTest.hpp"
#include "test/GUI/Client/Core/NBrowserTest.hpp"
#include "test/GUI/Client/Core/NLinkTest.hpp"
#include "test/GUI/Client/Core/NRootTest.hpp"
#include "test/GUI/Client/Core/NTreeTest.hpp"
#include "test/GUI/Client/Core/PropertyModelTest.hpp"
#include "test/GUI/Client/Core/TreeNodeTest.hpp"

using namespace CF::Common;
using namespace CF::UI::ClientTest;

int main(int argc, char * argv[])
{
  QCoreApplication app(argc, argv);
  int passed = 0;

  CF::Common::ExceptionManager::instance().ExceptionDumps = false;
  CF::Common::ExceptionManager::instance().ExceptionAborts = false;
  CF::Common::ExceptionManager::instance().ExceptionOutputs = false;

  CF::AssertionManager::instance().AssertionThrows = true;

  // CommonFunctionsTest must be the first to be run !!
  passed += QTest::qExec(new CommonFunctionsTest(), argc, argv);

  passed += QTest::qExec(new CommitDetailsTest(), argc, argv);
  passed += QTest::qExec(new CNodeTest(), argc, argv);
  passed += QTest::qExec(new NBrowserTest(), argc, argv);
  passed += QTest::qExec(new NTreeTest(), argc, argv);
  passed += QTest::qExec(new NLinkTest(), argc, argv);
  passed += QTest::qExec(new NRootTest(), argc, argv);
  passed += QTest::qExec(new PropertyModelTest(), argc, argv);
  passed += QTest::qExec(new TreeNodeTest(), argc, argv);

  return passed;
}
