// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_OptionPanel_h
#define CF_GUI_Client_UI_OptionPanel_h

////////////////////////////////////////////////////////////////////////////////

#include <QDomNamedNodeMap>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QObject>
#include <QWidget>

#include "Common/Option.hpp"

#include "GUI/Client/Core/OptionType.hpp"

#include "GUI/Client/UI/LibClientUI.hpp"

class QDomNodeList;
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

namespace ClientCore {
  class CommitDetails;
  struct NodeOption;
}

namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  class GraphicalValue;
  struct CloseConfirmationInfos;

  /// @brief Panel to m_view and modify m_options of an object.

  /// This class allows user to display and modify options of an object.

  /// @author Quentin Gasper.

  class ClientUI_API OptionPanel : public QWidget
  {
    Q_OBJECT

  public:
    /// @brief Constructor.

    /// Builds an @c OptionPanel with no options. The panel is neither in
    /// read-only mode nor advanced mode.
    /// @param parent The parent widget. Default value is @c nullptr
    OptionPanel(QWidget * parent = nullptr);

    /// @brief Destructor.

    /// Frees the allocated memory.  Parent is not destroyed.
    ~OptionPanel();
    
    /// @brief Destroys all graphical values.
    void clearOptions();

    /// @brief Indicates wether at least on option has been modified.

    /// @return Returns @c true if at least one option has been modified.
    bool isModified() const;

    /// @brief Build containers with modified m_options.

    /// This method allows to get old and new values of each modified option.
    /// The old value is the original one, that the option had on calling
    /// @c setOptions. The new value is the current option value. All
    /// intermediate values (i.e. : if user modified several times the same
    /// option) are ignored. These values are stored in @c oldValues and
    /// @c newValues respectively. Each modified option name is stored in the
    /// provided string list. Hash map keys have one of these names. @n @n
    ///
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
    void getModifiedOptions(ClientCore::CommitDetails & commitDetails) const;

    void addOption(CF::Common::Option::ConstPtr option);

  signals:

    void valueChanged();

  private:

    /// @brief List containing basic m_options components.
    QList<GraphicalValue *> m_options;

    /// @brief Main layout containing all option widgets.
    QFormLayout * m_mainLayout;

     /// @brief Indicates if the panel is in advanced mode or not.

    /// If @c true, the panel is in advanced mode. Advanced m_options (if any)
    /// are displayed. Otherwise, they are m_hidden.
    bool m_advancedMode;

    /// @brief Puts all modified options in a provided hashmap.

    /// Only modified options will be set, meaning that the hashmap may be empty
    /// if no option has been modified. The map is cleared before first use.
    /// @param options A hashmap were modified options will be written. The
    /// key is the option name and the value is the option new value.
    void getOptions(QMap<QString, QString> & options) const;

    /// @brief Build containers with modified options.

    /// This method allows to get old and new values of each modified option.
    /// The old value is the original one, that the option had on calling
    /// @c setOptions. The new value is the current option value. All
    /// intermediate values (i.e. : if user modified several times the same
    /// option) are ignored.
    /// @param graphicalOptions Graphical components corresponding the option
    /// nodes.
    /// @param commitDetails Reference to where meodified options will be stored.
    /// @b Not cleared before first use.
    void getModifiedOptions(const QList<GraphicalValue *> & graphicalOptions,
                            ClientCore::CommitDetails & commitDetails) const;

    /// @brief Checks if options has been modified.

    /// @param graphicalOptions Options to check
    /// @return Returns @c true if at least one option has been modified;
    /// otherwise, returns @c false.
    bool isModified(const QList<GraphicalValue *> & graphicalOptions) const;

    void setOptions(const QList<CF::Common::Option::ConstPtr> & list);

  }; // class OptionPanel

  /////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_OptionPanel_h
