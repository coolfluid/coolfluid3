// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_JournalBrowser_hpp
#define cf3_ui_core_JournalBrowser_hpp

////////////////////////////////////////////////////////////////////////////

#include <QAbstractItemModel>
#include <QList>

#include "common/XML/XmlNode.hpp"

#include "ui/core/CNode.hpp"

class QModelIndex;
class QString;
class QStringList;
class QVariant;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common {
namespace XML {
  class XmlDoc;
}
}

namespace ui {
namespace core {

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

  NJournalBrowser(const common::XML::XmlNode * rootNode, QObject * parent = 0);

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
  virtual QString tool_tip() const;

  //@} END VIRTUAL FUNCTIONS

  const common::SignalArgs & signal(const QModelIndex & index) const;

  void set_root_node(const common::XML::XmlNode * root_node);

  void request_journal();

  void list_journal(common::SignalArgs & node);

  void send_exec_signal(const QModelIndex & index);

signals:

  void update_view();

protected:

  /// Disables the local signals that need to.
  /// @param localSignals Map of local signals. All values are set to true
  /// by default.
  virtual void disable_local_signals(QMap<QString, bool> & localSignals) const {}

private: // data

  QStringList m_columns;

  common::XML::XmlNode m_root_node;

  QList<common::SignalArgs *> m_children;

  boost::shared_ptr<common::XML::XmlDoc> m_doc;

  /// @brief Converts an index to a signal node

  /// @param index Node index to convert
  /// @return Returns the tree node, or @c nullptr if the index could
  /// not be converted (i.e. index is invalid)
  inline common::SignalArgs * index_to_xml_node(const QModelIndex & index) const
  {
    return static_cast<common::SignalArgs *>(index.internalPointer());
  }

  QString read_attribute( const common::SignalArgs & sig, const char * name) const;

  boost::shared_ptr<common::XML::XmlDoc> m_current_doc;

}; // JournalBrowser

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_JournalBrowser_hpp
