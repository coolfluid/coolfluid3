// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
