// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <QApplication>
#include <QtTest>

#include "common/CF.hpp"
#include "common/Assertions.hpp"
#include "common/Exception.hpp"

#include "test/ui/Graphics/GraphicalArrayTest.hpp"
#include "test/ui/Graphics/GraphicalBoolTest.hpp"
#include "test/ui/Graphics/GraphicalIntTest.hpp"
#include "test/ui/Graphics/GraphicalDoubleTest.hpp"
#include "test/ui/Graphics/GraphicalRestrictedListTest.hpp"
#include "test/ui/Graphics/GraphicalStringTest.hpp"
#include "test/ui/Graphics/GraphicalUintTest.hpp"
#include "test/ui/Graphics/GraphicalUriTest.hpp"
#include "test/ui/Graphics/GraphicalValueTest.hpp"

using namespace cf3::ui::GraphicsTest;

int main(int argc, char * argv[])
{
#ifdef Q_WS_X11
  if( getenv("DISPLAY") == 0 ) // if no graphical environment is found, we exit
  {
    std::cout << "No graphical environment found, exiting..." << std::endl;
    return 0;
  }
#endif

  QApplication app(argc, argv);
  int passed = 0;

  cf3::common::ExceptionManager::instance().ExceptionDumps = false;
  cf3::common::ExceptionManager::instance().ExceptionAborts = false;
  cf3::common::ExceptionManager::instance().ExceptionOutputs = false;

  cf3::common::AssertionManager::instance().AssertionThrows = true;

  passed += QTest::qExec(new GraphicalArrayTest(), argc, argv);
  passed += QTest::qExec(new GraphicalBoolTest(), argc, argv);
  passed += QTest::qExec(new GraphicalDoubleTest(), argc, argv);
  passed += QTest::qExec(new GraphicalIntTest(), argc, argv);
  passed += QTest::qExec(new GraphicalRestrictedListTest(), argc, argv);
  passed += QTest::qExec(new GraphicalStringTest(), argc, argv);
  passed += QTest::qExec(new GraphicalUintTest(), argc, argv);
  passed += QTest::qExec(new GraphicalUriTest(), argc, argv);
  passed += QTest::qExec(new GraphicalValueTest(), argc, argv);

  return passed;
}
