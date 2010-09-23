// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>

#include "Common/CF.hpp"

#include "GUI/Client/Core/NMesh.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

NMesh::NMesh(const QString & name)
  : CNode(name, "CMesh", MESH_NODE)
{
  BUILD_COMPONENT;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NMesh::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Drive);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NMesh::getToolTip() const
{
  return this->getComponentType();
}
