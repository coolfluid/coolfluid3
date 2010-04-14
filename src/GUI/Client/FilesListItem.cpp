#include <QtCore>
#include <stdexcept>

#include "GUI/Client/UnknownType.hpp"

#include "GUI/Client/FilesListItem.hpp"

using namespace CF::GUI::Client;

FilesListItem::FilesListItem(const QIcon & icon, const QString & text,
                             FilesListItemType type)
: QStandardItem(icon, text)
{
  if(type != DIRECTORY && type != FILE)
    throw UnknownType(FromHere(), "Unknown item type");

  m_type = type;

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FilesListItemType FilesListItem::getType() const
{
  return m_type;
}
