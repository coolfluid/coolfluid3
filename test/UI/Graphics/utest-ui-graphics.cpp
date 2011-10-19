// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <QApplication>
#include <QtTest>

#include "Common/CF.hpp"
#include "Common/Assertions.hpp"
#include "Common/Exception.hpp"

#include "test/UI/Graphics/GraphicalArrayTest.hpp"
#include "test/UI/Graphics/GraphicalBoolTest.hpp"
#include "test/UI/Graphics/GraphicalIntTest.hpp"
#include "test/UI/Graphics/GraphicalDoubleTest.hpp"
#include "test/UI/Graphics/GraphicalRestrictedListTest.hpp"
#include "test/UI/Graphics/GraphicalStringTest.hpp"
#include "test/UI/Graphics/GraphicalUintTest.hpp"
#include "test/UI/Graphics/GraphicalUriTest.hpp"
#include "test/UI/Graphics/GraphicalValueTest.hpp"

using namespace cf3::UI::GraphicsTest;

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
