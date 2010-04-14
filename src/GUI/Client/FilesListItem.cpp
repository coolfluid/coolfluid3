#include <QtCore>
#include <stdexcept>

#include "GUI/Client/UnknownTypeException.hh"

#include "GUI/Client/FilesListItem.hh"

using namespace CF::GUI::Client;

FilesListItem::FilesListItem(const QIcon & icon, const QString & text,
                             FilesListItemType type)
: QStandardItem(icon, text)
{
  if(type != DIRECTORY && type != FILE)
    throw UnknownTypeException(FromHere(), "Unknown item type");
  
  m_type = type;
  
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FilesListItemType FilesListItem::getType() const
{
  return m_type;
}
