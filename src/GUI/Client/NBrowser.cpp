#include <QtCore>
#include <QtGui>

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/NBrowser.hpp"

using namespace CF::GUI::Client;

NBrowser::NBrowser()
  : CNode(CLIENT_BROWSERS, "NBrowser", CNode::BROWSER_NODE),
    m_counter(0)
{
  BUILD_COMPONENT;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NBrowser::generateName()
{
 return QString("Browser_%1").arg(m_counter++);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NBrowser::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Folder);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NBrowser::getToolTip() const
{
  return this->getComponentType();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NBrowser::getOptions(QList<NodeOption> & params) const
{
  QHash<QString, NodeOption>::const_iterator it = m_options.begin();

  params.clear();

  for( ; it != m_options.end() ; it++)
    params.append(it.value());
}
