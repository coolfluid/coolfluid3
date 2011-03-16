// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"

#include "GUI/Client/UI/FilesListItem.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

//////////////////////////////////////////////////////////////////////////

FilesListItem::FilesListItem(const QIcon & icon, const QString & text,
                             FilesListItemType type)
: QStandardItem(icon, text)
{
  if(type != DIRECTORY && type != FILE)
    throw CF::Common::ValueNotFound(FromHere(), "Unknown item type");

  m_type = type;
}

//////////////////////////////////////////////////////////////////////////

FilesListItemType FilesListItem::getType() const
{
  return m_type;
}

//////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
