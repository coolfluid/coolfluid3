#include <QFileIconProvider>

#include "GUI/Client/NArray.hpp"

using namespace CF::GUI::Client;

NArray::NArray(const QString & name) :
    CNode(name, "CArray", ARRAY_NODE)
{
  BUILD_COMPONENT;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NArray::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::File);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NArray::getToolTip() const
{
  return this->getComponentType();
}

