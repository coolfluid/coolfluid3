// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>
#include <QStringList>

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/UI/FilesListItem.hpp"
#include "GUI/Client/UI/NRemoteOpen.hpp"

using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

NRemoteOpen::NRemoteOpen(QMainWindow * parent)
: NRemoteBrowser("NRemoteOpen", parent)
{
  BUILD_COMPONENT;

  this->setIncludeFiles(true);
  this->setIncludeNoExtension(true);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NRemoteOpen::~NRemoteOpen()
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NRemoteOpen::Ptr NRemoteOpen::create(QMainWindow * parent)
{
  NRemoteOpen::Ptr rop(new NRemoteOpen(parent));

  ClientRoot::browser()->addNode(rop);

  return rop;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NRemoteOpen::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::File);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NRemoteOpen::getToolTip() const
{
  return this->getComponentType();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ValidationPolicy NRemoteOpen::isAcceptable(const QString & name, bool isDir)
{
  if(isDir)
    return POLICY_ENTER_DIRECTORY;

  m_fileList << name;
  return POLICY_VALID;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ValidationPolicy NRemoteOpen::isAcceptable(const QStringList & names)
{
  QStringList::const_iterator it = names.begin();
  ValidationPolicy validation = POLICY_VALID;

  while(it != names.end() && validation == POLICY_VALID)
  {
    QString item = *it;

    if(this->isDirectory(item) && names.size() > 1)
    {
      this->showError("Directories are not allowed in multiple selection.");
      validation = POLICY_NOT_VALID;
      m_fileList.clear();
    }

    else if(this->isDirectory(item) && names.size() == 1)
      validation = POLICY_ENTER_DIRECTORY;

    else
      m_fileList << item;

    it++;
  }

  return validation;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NRemoteOpen::getSelectedFile() const
{
  if(!m_fileList.isEmpty())
    return m_fileList.at(0);

  return QString();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//QStringList NRemoteOpen::getSelectedFileList() const
//{
//  return m_fileList;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteOpen::reinitValues()
{
  m_fileList.clear();
}
