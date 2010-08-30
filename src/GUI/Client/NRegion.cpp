#include <QFileIconProvider>

#include "GUI/Client/NRegion.hpp"

using namespace CF::GUI::Client;

NRegion::NRegion(const QString & name) :
    CNode(name, "CRegion", REGION_NODE)
{
  BUILD_COMPONENT;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NRegion::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::File);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NRegion::getToolTip() const
{
  return this->getComponentType();
}

