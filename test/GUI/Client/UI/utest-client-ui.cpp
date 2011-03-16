// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QApplication>
#include <QtTest>

int main(int argc, char * argv[])
{
  QApplication app(argc, argv);
  int passed = 0;

//  CF::Common::ExceptionManager::instance().ExceptionDumps = false;
//  CF::Common::ExceptionManager::instance().ExceptionAborts = false;
//  CF::Common::ExceptionManager::instance().ExceptionOutputs = false;

//  CF::AssertionManager::instance().AssertionThrows = true;

  return passed;
}
