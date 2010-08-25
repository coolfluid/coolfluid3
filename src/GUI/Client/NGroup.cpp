#include <QFileIconProvider>

#include "GUI/Client/NGroup.hpp"

using namespace CF::GUI::Client;

NGroup::NGroup(const QString & name) :
    CNode(name, "CGroup", GROUP_NODE)
{
  BUILD_COMPONENT;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NGroup::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Folder);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NGroup::getToolTip() const
{
  return this->getComponentType();
}

