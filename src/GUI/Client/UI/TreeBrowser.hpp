// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_TreeBrowser_hpp
#define CF_GUI_Client_UI_TreeBrowser_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QWidget>
#include <QMap>

#include "GUI/Client/UI/LibClientUI.hpp"

class QAction;
class QGridLayout;
class QMenu;
class QModelIndex;
class QPersistentModelIndex;
class QPushButton;
class QLineEdit;
class QToolBar;
class QToolButton;
class QVBoxLayout;

template<typename T> class QList;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  ///////////////////////////////////////////////////////////////////////////

  class TreeView;

  /// @brief This widget allows allows to browse the treeview in a special way.

  /// Each time a user double-clicks on an node, it becomes the new
  /// treeview root, thus only its children are visible.@n

  /// The hitory is maintained as a list of @c QPersistentModelIndex, in which
  /// the last item is the last node the user clicked on. An integer value,
  /// constantly indicates the current root index in the tree.@n

  /// The widget provides two buttons: one to go backward (decrease the
  /// current index by 1) through the history, and another one to go forward
  /// (increase the current index by 1). "Back" and "Forward" direction
  /// are represented as black arrows (grayed when button is disabled).@n

  /// Each button provides a menu that allows to jump over more than one
  /// item in one click. The "back" menu displays all paths that are
  /// older than the current index in the history. The "forward" menu
  /// displays those that are newer. The current index is never displayed
  /// in the menus.@n

  /// Each time the tree is double-clicked, the node on which user clicked
  /// is appended to the history. If the current index is not the last item
  /// in the history, all items after it are removed before the new one is
  /// inserted. This is to make sure that the new item is the last of the
  /// list @b and is directly after the current index in the history. After
  /// insterting, the new item becomes the current index.@n

  class ClientUI_API TreeBrowser : public QWidget
  {
    Q_OBJECT

  public:

    /// @brief Constructor

    /// @param view The observed treeview. Can not be @c nullptr.
    /// @param parent Widget parent.
    TreeBrowser(TreeView * view, QWidget *parent = 0);

    /// @brief Destructor

    /// Frees all allocated memory. The treeview and the parent are not
    /// deleted.
    ~TreeBrowser();

  public slots:

    void focusFilter();

  private slots:

    /// @brief Slot called when user clicks on the @e previous button.
    /// The current index is decreased by one and the treeview is updated.
    void previousClicked();

    /// @brief Slot called when user clicks on the @e next button.
    /// The current index is increased by one and the treeview is updated.
    void nextClicked();

    /// @brief Slot called when user double-clicks on a node.
    void doubleClicked(const QModelIndex & index);

    /// @brief Slot called when a menu item is clicked.
    void actionTriggered();

    void filterUpdated(const QString & text);

  private:

    /// @brief The observed treeview.
    TreeView * m_treeView;

    /// @brief Previous button
    QToolButton * m_btPrevious;

    /// @brief Next button
    QToolButton * m_btNext;

    /// @brief The history
    QList<QPersistentModelIndex> m_history;

    QLineEdit * m_filter;

    /// @brief Menu actions

    /// The key is the action. The value is the index of the corresponding
    /// index.
    QMap<QAction *, int> m_actions;

    /// @brief The current index.
    int m_currentIndex;

    /// @brief Updates buttons "enabled" state and rebuilds the menus.
    void updateButtons();

    /// @brief Buttons layout.
    QGridLayout * m_buttonsLayout;

    /// @brief Main layout.
    QVBoxLayout * m_mainLayout;

    /// @brief The menu for the previous button
    QMenu * m_menuPrevious;

    /// @brief The menu for the next button
    QMenu * m_menuNext;
  }; // class TreeBrowser

  ///////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_TreeBrowser_hpp
