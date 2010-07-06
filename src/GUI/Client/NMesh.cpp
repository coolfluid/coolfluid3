#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"

#include "GUI/Client/NMesh.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NMesh::getOptions(QList<NodeOption> & params) const
{
  params = m_options;
}
