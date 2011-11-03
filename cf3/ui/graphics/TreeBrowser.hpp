// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_TreeBrowser_hpp
#define cf3_ui_Graphics_TreeBrowser_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QWidget>
#include <QMap>

#include "ui/graphics/LibGraphics.hpp"

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

namespace cf3 {
namespace ui {
namespace graphics {

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

  class Graphics_API TreeBrowser : public QWidget
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

    void focus_filter();

  private slots:

    /// @brief Slot called when user clicks on the @e previous button.
    /// The current index is decreased by one and the treeview is updated.
    void previous_clicked();

    /// @brief Slot called when user clicks on the @e next button.
    /// The current index is increased by one and the treeview is updated.
    void next_clicked();

    /// @brief Slot called when user double-clicks on a node.
    void double_clicked(const QModelIndex & index);

    /// @brief Slot called when a menu item is clicked.
    void action_triggered();

    void filter_updated(const QString & text);

  private: // data

    /// @brief The observed treeview.
    TreeView * m_tree_view;

    /// @brief Previous button
    QToolButton * m_bt_previous;

    /// @brief Next button
    QToolButton * m_bt_next;

    /// @brief The history
    QList<QPersistentModelIndex> m_history;

    QLineEdit * m_filter;

    /// @brief Menu actions

    /// The key is the action. The value is the index of the corresponding
    /// index.
    QMap<QAction *, int> m_actions;

    /// @brief The current index.
    int m_current_index;

    /// @brief Buttons layout.

    QGridLayout * m_buttons_layout;

    /// @brief Main layout.
    QVBoxLayout * m_main_layout;

    /// @brief The menu for the previous button
    QMenu * m_menu_previous;

    /// @brief The menu for the next button
    QMenu * m_menu_next;

  private: // functions

    /// @brief Updates buttons "enabled" state and rebuilds the menus.
    void update_buttons();
  }; // class TreeBrowser

  ///////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_TreeBrowser_hpp
