// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>
#include <QStringList>

#include "UI/Core/NBrowser.hpp"
#include "UI/Graphics/FilesListItem.hpp"
#include "UI/Graphics/NRemoteOpen.hpp"

using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

NRemoteOpen::NRemoteOpen(QMainWindow * parent)
: NRemoteBrowser("NRemoteOpen", parent)
{
  this->set_include_files(true);
  this->set_include_no_extension(true);
}

//////////////////////////////////////////////////////////////////////////

NRemoteOpen::~NRemoteOpen()
{
}

//////////////////////////////////////////////////////////////////////////

NRemoteOpen::Ptr NRemoteOpen::create(QMainWindow * parent)
{
  NRemoteOpen::Ptr rop(new NRemoteOpen(parent));

  NBrowser::global()->add_node(rop);

  return rop;
}

//////////////////////////////////////////////////////////////////////////

QString NRemoteOpen::tool_tip() const
{
  return this->component_type();
}

//////////////////////////////////////////////////////////////////////////

ValidationPolicy NRemoteOpen::is_acceptable(const QString & name, bool is_dir)
{
  if(is_dir)
    return POLICY_ENTER_DIRECTORY;

  m_file_list << name;
  return POLICY_VALID;
}

//////////////////////////////////////////////////////////////////////////

ValidationPolicy NRemoteOpen::is_acceptable(const QStringList & names)
{
  QStringList::const_iterator it = names.begin();
  ValidationPolicy validation = POLICY_VALID;

  while(it != names.end() && validation == POLICY_VALID)
  {
    QString item = *it;

    if(this->is_directory(item) && names.size() > 1)
    {
      this->show_error("Directories are not allowed in multiple selection.");
      validation = POLICY_NOT_VALID;
      m_file_list.clear();
    }

    else if(this->is_directory(item) && names.size() == 1)
      validation = POLICY_ENTER_DIRECTORY;

    else
      m_file_list << item;

    it++;
  }

  return validation;
}

//////////////////////////////////////////////////////////////////////////

QString NRemoteOpen::selected_file() const
{
  if(!m_file_list.isEmpty())
    return m_file_list.at(0);

  return QString();
}

//////////////////////////////////////////////////////////////////////////

//QStringList NRemoteOpen::getSelectedFileList() const
//{
//  return m_fileList;
//}

//////////////////////////////////////////////////////////////////////////

void NRemoteOpen::reinit_values()
{
  m_file_list.clear();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
