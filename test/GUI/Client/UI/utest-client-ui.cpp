// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <QApplication>
#include <QtTest>

#include "Common/CF.hpp"
#include "Common/Exception.hpp"

#include "test/GUI/Client/UI/GraphicalBoolTest.hpp"
#include "test/GUI/Client/UI/GraphicalIntTest.hpp"
#include "test/GUI/Client/UI/GraphicalDoubleTest.hpp"
#include "test/GUI/Client/UI/GraphicalStringTest.hpp"
#include "test/GUI/Client/UI/GraphicalUintTest.hpp"
#include "test/GUI/Client/UI/GraphicalUriTest.hpp"
#include "test/GUI/Client/UI/GraphicalValueTest.hpp"

using namespace CF::UI::ClientTest;

int main(int argc, char * argv[])
{
#ifdef Q_WS_X11
  if( getenv("DISPLAY") == 0 ) // if no graphical environment is found, we exit
  {
    std::cout << "No graphical enironment found, exiting..." << std::endl;
    return 0;
  }
#endif

  QApplication app(argc, argv);
  int passed = 0;

  CF::Common::ExceptionManager::instance().ExceptionDumps = false;
  CF::Common::ExceptionManager::instance().ExceptionAborts = false;
  CF::Common::ExceptionManager::instance().ExceptionOutputs = false;

  CF::AssertionManager::instance().AssertionThrows = true;

  passed += QTest::qExec(new GraphicalBoolTest(), argc, argv);
  passed += QTest::qExec(new GraphicalDoubleTest(), argc, argv);
  passed += QTest::qExec(new GraphicalIntTest(), argc, argv);
  passed += QTest::qExec(new GraphicalStringTest(), argc, argv);
  passed += QTest::qExec(new GraphicalUintTest(), argc, argv);
  passed += QTest::qExec(new GraphicalUriTest(), argc, argv);
  passed += QTest::qExec(new GraphicalValueTest(), argc, argv);

  return passed;
}
