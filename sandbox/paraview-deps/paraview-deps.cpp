// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QApplication>

#include <pqApplicationCore.h>
#include <pqTreeView.h>

int main(int argc, char * argv[])
{
  QApplication app(argc, argv);

  pqApplicationCore appCore(argc, argv);

  pqTreeView * tv = new pqTreeView();

  tv->show();

  return app.exec();
}
