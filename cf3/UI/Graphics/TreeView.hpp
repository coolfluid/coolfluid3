// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_TreeView_h
#define cf3_ui_Graphics_TreeView_h

////////////////////////////////////////////////////////////////////////////////

#include <QTreeView>

#include "UI/Graphics/LibGraphics.hpp"

class QMainWindow;
class QMenu;
class QMenuBar;
class QModelIndex;
class QSortFilterProxyModel;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common { class URI; }

namespace ui {

namespace core { struct TSshInformation; }

namespace graphics {

////////////////////////////////////////////////////////////////////////////////

  class FilteringModel;
  class CentralPanel;
  class SignalManager;

  /////////////////////////////////////////////////////////////////////////////

  /// @brief This class manages the tree view.

  /// @author Gasper Quentin

  class Graphics_API TreeView : public QTreeView
  {
    Q_OBJECT

  public:
    /// @brief Constructor.

    /// @param optionsPanel Panel options() of the selected node will be displayed.
    /// @param parent Parent window. May be @c nullptr.
    /// @throws std::invalid_argument if @c optionsPanel is @c nullptr.
    TreeView( CentralPanel * optionsPanel,
              QMainWindow * parent = nullptr,
              bool contextMenuAllowed = true);

    /// @brief Destructor.

    /// Frees all allocated memory. Neither the options() panel nor the parent
    /// are destroyed.
    ~TreeView();

    /// @brief Changes the read-only mode.

    /// @param readOnly If @c true, the tree m_view will switch to the read-only
    /// mode.
    void set_read_only(bool readOnly);

    /// @brief Indicates whether the tree m_view is in read-only mode or not.

    /// @return Returns @c true if the tree m_view is in read-only mode.
    /// Otherwise, returns @c false.
    bool is_read_only() const;

    common::URI selected_path() const;

    common::URI path_from_index(const QModelIndex & index);

    QIcon icon_from_index(const QModelIndex & index);

    void select_item(const cf3::common::URI & path);

    void set_filter(const QString & pattern);

    bool try_commit();

  protected:

    /// @brief Method called when a mouse button is pressed in the Client.

    /// This method overloads parent class method. Four cases are possible :
    /// @li If user right-clicks, a context menu is displayed.
    /// @li If user left-clicks on another node than the currently selected one
    /// @b and @c confirmChangeOptions() returns @c true, options() in the
    /// options() panel are changed.
    /// @li If user left-clicks on the selected node nothing is done.
    /// @li Middle button has no effect.
    /// @param event Event that occured.
    virtual void mousePressEvent(QMouseEvent * event);

    virtual void mouseDoubleClickEvent(QMouseEvent * event);

    virtual void keyPressEvent(QKeyEvent * event);

  private slots:

    void current_index_changed(const QModelIndex & newIndex, const QModelIndex & oldIndex);

  signals:

    void item_double_clicked(const QModelIndex & index);

  private:

    /// @brief Panel used to display and modify options() for a selected
    /// object.
    CentralPanel * m_central_panel;

    /// @brief List of abstract types
    QStringList m_abstract_types;

    /// @brief Filter for the Client.

    /// Allows to switch between basic/advanced mode. The filter is used as the
    /// Client model. Its source is the tree model.
    FilteringModel * m_model_filter;

    /// @brief Indicates whether the tree m_view is in read-only mode or not.

    /// If @c true, the tree m_view is read-only mode. When it is read-only mode,
    /// all options() in the context that may modify an object are disbaled.
    /// "Delete", "Rename", "Add a child node" and "Add an option" m_items are then
    /// disabled.
    bool m_read_only;

    SignalManager * m_signal_manager;

    bool m_context_menu_allowed;

    /// @brief Asks user to commit or rollback before changing options() in
    /// options() panel.

    /// If modifications were committed, nothing is asked and the method
    /// immediately returns @c true. If the commit is requested by the user,
    /// it is processed by this method.
    /// @param index Node index on which user clicked. If it is equals to
    /// @c #currentIndexInPanel nothing is asked and the method
    /// immediately returns @c true.
    /// @param okIfSameIndex If @c false, the method checks if indexes are the
    /// same. If @c true, no check is done.
    /// @return Returns @c false if the user clicked on "Cancel" ; otherwise
    /// returns @c true.
    bool confirm_change_options(const QModelIndex & index, bool okIfSameIndex = false);

  }; // class TreeView

  /////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

  /////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_TreeView_h
