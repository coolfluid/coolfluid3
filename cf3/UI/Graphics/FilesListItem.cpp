// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BasicExceptions.hpp"

#include "UI/Graphics/FilesListItem.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

FilesListItem::FilesListItem(const QIcon & icon, const QString & text,
                             FilesListItemType type)
: QStandardItem(icon, text)
{
  if(type != DIRECTORY && type != FILE)
    throw cf3::common::ValueNotFound(FromHere(), "Unknown item type");

  m_type = type;
}

//////////////////////////////////////////////////////////////////////////

FilesListItemType FilesListItem::get_type() const
{
  return m_type;
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
