#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"
#include "Common/CPath.hpp"

#include "GUI/Client/NLink.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

NLink::NLink(const QString & name, const CPath & targetPath)
  : CNode(name, "CLink"),
    m_targetPath(targetPath)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QList<NodeAction> NLink::getNodeActions() const
{
  static QList<NodeAction> list;

  return list;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NLink::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Network);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLink::getToolTip() const
{
  return QString("Target: %1").arg(m_targetPath.string().c_str());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLink::getClassName() const
{
  return "NLink";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::getParams(QList<NodeParams> & params) const
{
//  CNode::Ptr target = Client
  params = m_params;
}
