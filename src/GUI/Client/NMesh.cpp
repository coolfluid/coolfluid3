#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"

#include "GUI/Client/NMesh.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

NMesh::NMesh(const QString & name)
  : CNode(name, "CMesh")
{

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

void NMesh::getOptions(QList<NodeOption> & params) const
{
  params = m_options;
}
