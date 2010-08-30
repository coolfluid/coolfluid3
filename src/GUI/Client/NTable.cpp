#include <QFileIconProvider>

#include "GUI/Client/NTable.hpp"

using namespace CF::GUI::Client;

NTable::NTable(const QString & name) :
    CNode(name, "CTable", TABLE_NODE)
{
  BUILD_COMPONENT;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NTable::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::File);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NTable::getToolTip() const
{
  return this->getComponentType();
}

