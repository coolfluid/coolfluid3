// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
