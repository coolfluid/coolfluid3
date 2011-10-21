// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QString>
#include <QtTest>

#include "UI/Core/NBrowser.hpp"

#include "test/UI/Core/NBrowserTest.hpp"

using namespace cf3::UI::Core;
using namespace cf3::UI::CoreTest;

void NBrowserTest::test_generateName()
{
  NBrowser b;

  QCOMPARE(b.generate_name(), QString("Browser_0"));
  QCOMPARE(b.generate_name(), QString("Browser_1"));
  QCOMPARE(b.generate_name(), QString("Browser_2"));

  for(int i = 0 ; i < 15 ; i++)
    b.generate_name();

  QCOMPARE(b.generate_name(), QString("Browser_18"));
}
