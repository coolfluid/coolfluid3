#include <QFileIconProvider>

#include "GUI/Client/NElements.hpp"

using namespace CF::GUI::Client;

NElements::NElements(const QString & name) :
    CNode(name, "CElements", ELEMENTS_NODE)
{
  BUILD_COMPONENT;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NElements::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::File);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NElements::getToolTip() const
{
  return this->getComponentType();
}

