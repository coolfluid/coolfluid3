#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"

#include "GUI/Client/NMesh.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

NMesh::NMesh(const QString & name)
  : CNode(name, "CLink")
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QList<NodeAction> NMesh::getNodeActions() const
{
  static QList<NodeAction> list;

  return list;
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

QString NMesh::getClassName() const
{
  return "NMesh";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NMesh::getParams(QList<NodeParams> & params) const
{
  params = m_params;
}
