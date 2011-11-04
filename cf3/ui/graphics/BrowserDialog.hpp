// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_BrowserDialog_hpp
#define cf3_ui_Graphics_BrowserDialog_hpp

#include <QDialog>

#include "ui/uicommon/LogMessage.hpp"

#include "ui/graphics/LibGraphics.hpp"

class QComboBox;
class QCompleter;
class QDialogButtonBox;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListView;
class QModelIndex;
class QPushButton;
class QSortFilterProxyModel;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

namespace core { class NRemoteFSBrowser; }

namespace graphics {

class FileFilter;

////////////////////////////////////////////////////////////////////////////

/// Provides graphical services for browsing a remote filesystem based by using
/// and instance of @c #NRemoteFSBrowser class.

/// @author Quentin Gasper.
class Graphics_API BrowserDialog : public QDialog
{
  Q_OBJECT

public:

  /// Contructor.
  /// @param parent Parent object. Can be null.
  BrowserDialog ( QWidget *parent = 0 );

  /// Shows the dialog.

  /// This is a blocking method. It will not return until the dialog is hidden.
  /// @param multi_select If @c true, user will be able to select multiple
  /// items at the same time.
  /// @param selected After the method returns, if @c multi_select was @c false,
  /// contains a @c QString with the path to the selected file. If @c multi_select
  /// was @c true, contains a @c QStringList with the paths to selected files.
  /// If user cancels the diolog, the parameter is not modified.
  /// @return Returns @c true if user clicked on "OK" button; otherwise, returns
  /// @c false.
  bool show ( bool multi_select, QVariant & selected );

protected:

  /// @brief Method called when user presses a key or a combination of keys

  /// This method overrides base class method and calls it before any other
  /// treatement. Depending on what is pressed, the method has 3 working modes:
  /// @li If @e Enter key is pressed and one of the buttons has the focus, it is
  /// like if user clicked on this button;
  /// @li If either no modifier key (such as ctrl, shift, alt, etc...) is
  /// pressed or shift key and another key are pressed, the combination is
  /// appended to the filter and @c #editFilter is focused in. To
  /// avoid confusion, list m_view is focused out.
  /// @li In all other cases, nothing is done. The method calls base class
  /// method.
  /// @param event Event that occured.
  virtual void keyPressEvent ( QKeyEvent * event );

  /// @brief Method called when user presses "Tab" key.

  /// This methods overrides base class method. If the path text edit has
  /// the focus, the completer is displayed, the first item is selected and
  /// appended to the path. If the path text edit does not have the focus,
  /// the base method is called.
  virtual bool focusNextPrevChild ( bool next );

private slots:

  /// Slot called when user edits the filter.
  /// @param new_text The new text.
  void filter_edited ( const QString & new_text );

  /// Slot called when user double-clicks on an item in the view.
  /// @param index The index of the item that was double=clicked.
  void double_clicked ( const QModelIndex & index );

  /// Slot called when the current path in the underlying model has changed.
  /// @param path The new current path.
  void current_path_changed ( const QString & path );

  /// Slot called when an element has been selected in the completer.
  /// @param text The selected text.
  void completer_activated ( const QString & text );

  /// Slot called when user edits the path.
  /// @param path The new path.
  void path_edited ( const QString & text );

  /// Slot called when has been added to the log.

  /// A messaged box is showed with the message.
  /// @param message The message.
  /// @param type The message type.
  void message  ( const QString& message , uiCommon::LogMessage::Type type );

private: // data

  /// The underlying model.
  boost::shared_ptr<core::NRemoteFSBrowser> m_model;

  /// The filtering model. It lies between the model and the view and adds the
  /// icons management besides the filtering.
  FileFilter * m_filter_model;

  /// The view.
  QListView * m_view;

  /// The box that displays the dialog buttons.
  QDialogButtonBox * m_buttons;

  /// Main layout of the dialog.
  QGridLayout * m_main_layout;

  /// View for the favorite places.
  QListView * m_favorites_view;

  /// Top layout for the graphical components related to the path.
  QHBoxLayout * m_path_layout;

  /// Label for the path.
  QLabel * m_lab_path;

  /// Line edit for the path.
  QLineEdit * m_edit_path;

  /// Layout for buttons related to favorites.
  QHBoxLayout * m_fav_buttons_layout;

  /// Button for adding a favorite.
  QPushButton * m_bt_add_fav;

  /// Button for removing a favorite.
  QPushButton * m_bt_remove_fav;

  /// Layout for the graphical components related to the filter.
  QHBoxLayout * m_filter_layout;

  /// Label for the filter.
  QLabel * m_lab_filter;

  /// Line edit for the filter.
  QLineEdit * m_edit_filter;

  /// Completer for the path completion.
  QCompleter * m_completer;

  /// Old path.
  QString m_old_path;

  /// If @c true, the completer is currently being updated.
  bool m_updating_completer;

private: // function

  /// Inits the GUI.
  void init_gui();

}; // BrowserDialog

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_BrowserDialog_hpp
