// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Core_JournalBrowser_hpp
#define CF_GUI_Core_JournalBrowser_hpp

////////////////////////////////////////////////////////////////////////////

#include <QAbstractItemModel>
#include <QList>

#include "UI/Core/CNode.hpp"

class QModelIndex;
class QString;
class QStringList;
class QVariant;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

class NJournalBrowser :
    public QAbstractItemModel,
    public CNode
{
  Q_OBJECT

public: // typedefs

  typedef boost::shared_ptr<NJournalBrowser> Ptr;
  typedef boost::shared_ptr<NJournalBrowser const> ConstPtr;

public:

  NJournalBrowser(const Common::XML::XmlNode * rootNode, QObject * parent = 0);

  /// @name VIRTUAL FUNCTIONS
  //@{

  /// @brief Implementation of @c QAbstractItemModel::data().

  /// Only the role @c Qt::DisplayRole and @c Qt::DecorationRole are accepted.
  /// Other roles will result to the return of an empty @c QVariant object
  /// (built with the default construtor).
  /// @param index Concerned item index.
  /// @param role Role of the returned value (only @c Qt::DisplayRole or
  /// @c Qt::DecorationRole).
  /// @return Returns an empty QVariant object if the role is neither
  /// @c Qt::DisplayRole nor @c Qt::DecorationRole or if the @c index.isValid()
  /// returns @c false. Otherwise, returns the nodename of the
  /// the item at the specified index.
  virtual QVariant data(const QModelIndex & index, int role) const;

  /// @brief Implementation of @c QAbstractItemModel::index().

  /// Gives the index of the item at the given row and column under
  /// the given parent. If the parent index is not valid, the root item
  /// is taken as parent.
  /// @param row Item row from the parent.
  /// @param column Item column.
  /// @param parent Item parent.
  /// @return Returns the requested index, or a nullptr index if
  /// <code>hasIndex(row, column, parent)</code> returns @c false.
  virtual QModelIndex index(int row, int column,
                            const QModelIndex & parent = QModelIndex()) const;

  /// @brief Implementation of @c QAbstractItemModel::parent().

  /// @param child Item index of which we would like to know the parent.
  /// @return Returns the parent index of the given child or a nullptr
  /// index if the child is not a valid index.
  virtual QModelIndex parent(const QModelIndex &child) const;

  /// @brief Implementation of @c QAbstractItemModel::rowCount().

  /// If the parent index is not valid, the root item is taken as parent.
  /// @return Returns the row count (number of children) of a given parent.
  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

  /// @brief Implementation of @c QAbstractItemModel::columnCount().
  /// @return Always returns 1.
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

  /// @brief Gives header titles.

  /// Overrides @c QAbstractItemModel::headerData().
  /// @param section Section number.
  /// @param orientation Header orientation.
  /// @param role Data role. Only @c Qt::DisplayRole is accepted.
  /// @return Returns the data or an empty @c QVariant on error.
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const;

  /// @brief Gives the tool tip text
  /// @return Returns The class name
  virtual QString toolTip() const;

  //@} END VIRTUAL FUNCTIONS

  const Common::SignalArgs & signal(const QModelIndex & index) const;

  void setRootNode(const Common::XML::XmlNode * rootNode);

  void requestJournal();

  void list_journal(Common::SignalArgs & node);

  void sendExecSignal(const QModelIndex & index);

signals:

  void updateView();

protected:

  /// Disables the local signals that need to.
  /// @param localSignals Map of local signals. All values are set to true
  /// by default.
  virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

private: // data

  QStringList m_columns;

  Common::XML::XmlNode m_rootNode;

  QList<Common::SignalArgs *> m_children;

  Common::XML::XmlDoc::Ptr m_doc;

  /// @brief Converts an index to a signal node

  /// @param index Node index to convert
  /// @return Returns the tree node, or @c nullptr if the index could
  /// not be converted (i.e. index is invalid)
  inline Common::SignalArgs * indexToXmlNode(const QModelIndex & index) const
  {
    return static_cast<Common::SignalArgs *>(index.internalPointer());
  }

  QString readAttribute( const Common::SignalArgs & sig, const char * name) const;

  Common::XML::XmlDoc::Ptr m_currentDoc;

}; // JournalBrowser

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Core_JournalBrowser_hpp
