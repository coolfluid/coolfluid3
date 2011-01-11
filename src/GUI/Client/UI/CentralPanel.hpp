// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_CentralPanel_h
#define CF_GUI_Client_UI_CentralPanel_h

////////////////////////////////////////////////////////////////////////////////

#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QObject>
#include <QWidget>

#include "Common/Option.hpp"

#include "GUI/Client/UI/LibClientUI.hpp"

class QFormLayout;
class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QModelIndex;
class QPushButton;
class QScrollArea;
class QSplitter;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

namespace ClientCore { class CommitDetails; }

namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  class GraphicalValue;
  class OptionLayout;
  struct CloseConfirmationInfos;

  /// @brief Panel to view and modify options of an object.

  /// This class allows user to display and modify options of an object or
  /// add new options.

  /// @author Quentin Gasper.

  class ClientUI_API CentralPanel : public QWidget
  {
    Q_OBJECT

  public:
    /// @brief Constructor.

    /// Builds an @c CentralPanel with no m_options. The panel is neither in
    /// read-only mode nor advanced mode.
    /// @param parent The parent widget. Default value is @c nullptr
    CentralPanel(QWidget * parent = nullptr);

    /// @brief Destructor.

    /// Frees the allocated memory.  Parent is not destroyed.
    ~CentralPanel();

    /// @brief Indicates wether at least on option has been modified.

    /// @return Returns @c true if at least one option has been modified.
    bool isModified() const;

    /// @brief Build containers with modified m_options.

    /// This method allows to get old and new values of each modified option
    /// (this does not include new m_options). The old value is the original one,
    /// that the option had on calling @c setOptions. The new value is the
    /// current option value. All intermediate values (i.e. : if user modified
    /// several times the same option) are ignored. These values are stored in
    /// @c oldValues and @c newValues respectively. Each modified option name
    /// is stored if the provides string list. Hash map keys have one of these
    /// names. @n @n
    /// The method garantees that:
    /// @li string list and hash map will have exactly the same number of
    /// elements
    /// @li all hash map keys can be found in the string list
    /// @li each string list item has a corresponding key in both hash maps.
    /// New m_options values are not stored in any hash map.
    ///
    /// To ensure consistency of the data returned, these four containers are
    /// cleared before first use.
    /// @param m_options String list where modified option names will be stored.
    /// @param newValues This hash map is used to store old value of an option.
    /// The key is the option name as stored in @c m_options string list. The
    /// value is the old value.
    /// @param newValues This hash map is used to store new value of an option.
    /// The key is the option name as stored in @c m_options string list. The
    /// value is the new value.
    /// @param m_newOptions String list where new option names will be stored.
    void modifiedOptions(ClientCore::CommitDetails & commitDetails) const;

    /// @brief Gives the current path.

    /// @return Returns the current path.
    QString currentPath() const;

  public slots:

    /// @brief Slot called when user clicks on "Commit changes" button.

    /// If at least one option has been modified, @c changesMade signal is
    /// emitted.
    void btApplyClicked();

  private slots:

    void currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex);

    void advancedModeChanged(bool advanced);

    void dataChanged(const QModelIndex & first, const QModelIndex & last);

    void btSeeChangesClicked();

    void btForgetClicked();

    void valueChanged();

  private:

    /// @brief Scroll area for basic m_options
    QScrollArea * m_scrollBasicOptions;

    /// @brief Scroll area for advanced m_options
    QScrollArea * m_scrollAdvancedOptions;

    /// @brief List containing basic m_options components.
    OptionLayout * m_basicOptionLayout;

    /// @brief List containing advanced m_options components.
    OptionLayout * m_advancedOptionLayout;

    /// @brief Button used to commit changes made.
    QPushButton * m_btApply;

    QPushButton * m_btForget;

    QPushButton * m_btSeeChanges;

    QGridLayout * m_buttonsLayout;

    /// @brief Main layout containing basic and advanced option panels.

    /// This layout is composed of three lines and one column.
    QGridLayout * m_mainLayout;

    QGridLayout * m_topLayout;

    /// @brief Groupbox used to display basic m_options components
    /// with a titled border.

    ///  Its m_layout is @c #basicOptionsLayout.
    QGroupBox * m_gbBasicOptions;

    /// @brief Groupbox used to display advanced m_options components
    /// with a titled border.

    ///  Its m_layout is @c #advancedOptionsLayout.
    QGroupBox * m_gbAdvancedOptions;

     /// @brief Indicates if the panel is in advanced mode or not.

    /// If @c true, the panel is in advanced mode. Advanced m_options (if any)
    /// are displayed. Otherwise, they are m_hidden.
    bool m_advancedMode;

    QString m_currentPath;

    QSplitter * m_splitter;

    bool m_modelReset;

    void setOptions(const QList<CF::Common::Option::ConstPtr> & list);

    void setButtonsVisible(bool visible);

    void setButtonsEnabled(bool enabled);

  }; // class CentralPanel

  /////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_CentralPanel_h
