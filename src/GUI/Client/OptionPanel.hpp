#ifndef CF_GUI_Client_OptionPanel_h
#define CF_GUI_Client_OptionPanel_h

////////////////////////////////////////////////////////////////////////////////

#include <QDomNamedNodeMap>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QObject>
#include <QWidget>

#include "GUI/Client/OptionType.hpp"

class QDomNodeList;
class QFormLayout;
class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QModelIndex;
class QPushButton;
class QScrollArea;
class QSplitter;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

////////////////////////////////////////////////////////////////////////////////

  class CommitDetails;
  class GraphicalOption;
  struct CloseConfirmationInfos;
  struct NodeOption;

  /// @brief Panel to m_view and modify m_options of an object.

  /// This class allows user to display and modify m_options of an object or
  /// add new m_options.

  /// @author Quentin Gasper.

  class OptionPanel : public QWidget
  {
    Q_OBJECT

  public:
    /// @brief Constructor.

    /// Builds an @c OptionPanel with no m_options. The panel is neither in
    /// read-only mode nor advanced mode.
    /// @param parent The parent widget. Default value is @c CFNULL
    OptionPanel(QWidget * parent = CFNULL);

    /// @brief Destructor.

    /// Frees the allocated memory.  Parent is not destroyed.
    ~OptionPanel();

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
    void getModifiedOptions(CommitDetails & commitDetails) const;

    /// @brief Gives the current path.

    /// @return Returns the current path.
    QString getCurrentPath() const;

  public slots:

    /// @brief Slot called when user clicks on "Commit changes" button.

    /// If at least one option has been modified, @c changesMade signal is
    /// emitted.
    void commitChanges() const;

  private slots:

    void currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex);

    void advancedModeChanged(bool advanced);

    void dataChanged(const QModelIndex & first, const QModelIndex & last);

    void readOnlyModeChanged(const QModelIndex & index, bool readOnly);

    void checkOptions();

    void resetChanges();

  signals:

    /// @brief Signal emitted when user clicks on "Commit changes" button if
    /// at least one option has been modified.

    /// @param modOptions XML document representing all modified m_options.
    /// Each document child is a modified option.
    /// @param m_newOptions XML document representing all new m_options. Each
    /// document child is a new option.
    void changesMade(const QDomDocument & modOptions,
                     const QDomDocument & m_newOptions) const;

  private:

    /// @brief Scroll area for basic m_options
    QScrollArea * m_scrollBasicOptions;

    /// @brief Scroll area for advanced m_options
    QScrollArea * m_scrollAdvancedOptions;

    /// @brief List containing basic m_options components.
    QList<GraphicalOption *> m_basicOptions;

    /// @brief List containing advanced m_options components.
    QList<GraphicalOption *> m_advancedOptions;

    /// @brief Button used to commit changes made.
    QPushButton * m_btCommitChanges;

    QPushButton * m_btResetOptions;

    QPushButton * m_btCheckChanges;

    QHBoxLayout * m_buttonsLayout;

    /// @brief Layout used to display basic m_options components.
    QFormLayout * m_basicOptionsLayout;

    /// @brief Layout used to display advanced m_options components.
    QFormLayout * m_advancedOptionsLayout;

    /// @brief Main m_layout containing all widgets.

    /// This m_layout is composed of two lines and one column.
    QGridLayout * m_mainLayout;

    /// @brief Groupbox used to display basic m_options components
    /// with a titled border.

    ///  Its m_layout is @c #basicOptionsLayout.
    QGroupBox * m_gbBasicOptions;

    /// @brief Groupbox used to display advanced m_options components
    /// with a titled border.

    ///  Its m_layout is @c #advancedOptionsLayout.
    QGroupBox * m_gbAdvancedOptions;

    /// @brief Indicates if the line edits are in read-only mode or not.

    /// If @c true, the panel is in read-only mode. Only m_options having
    /// @c dynamic attribute set to @c true are modifiable.
    bool m_readOnly;

    /// @brief Indicates if the panel is in advanced mode or not.

    /// If @c true, the panel is in advanced mode. Advanced m_options (if any)
    /// are displayed. Otherwise, they are m_hidden.
    bool m_advancedMode;

    QString m_currentPath;

    QSplitter * m_splitter;

    /// @brief Builds a Unix-like path string to the given node.

    /// The string begins with a slash followed by the root node name and
    /// all given node parent nodes names, seperated by slashed (like in a
    /// Unix path).
    /// @param node Node from which the path will be extracted.
    /// @return Returns the built strings.
    QString getNodePath(QDomNode & node);

    /// @brief Builds an XML document containing all modified m_options.

    /// First the basic m_options and then the advanced ones.
    /// @return Returns the built XML document.
    QDomDocument getOptions() const;

    /// @brief Clears the given list by deleting the @c TOption
    /// objects its elements point to.

    /// After calling this method, the list is empty.
    /// @param list The list to clear.
    void clearList(QList<GraphicalOption *> & list);

    /// @brief Builds a part (basic or advanced m_options) of the XML document
    /// returned by @c #getOptions() and @c #getNewOptions().

    /// This document is built by comparing original m_options nodes to
    /// corresponding m_options components, which may have different values.
    /// If the values differ, the node is considered to have been modified
    /// and components values are taken as new values. Only modified nodes
    /// are appended to the document, which means that the document may be
    /// empty (if no option has been modified).
    /// @param nodes Original m_options nodes.
    /// @param m_options Options components.
    /// @param document Document where built nodes will be stored. The
    /// presence of this parameter is due to the fact that a node can not
    /// exist if it does not belong to a document.
    void buildOptions(const QDomDocument & nodes,
                      const QList<GraphicalOption *> & options,
                      QDomDocument & document) const;

    /// @brief Applies the basic/advanced modes to the panel.

    /// For each node in the given XML document, the corresponding option
    /// components (in the given list) @c enabled property is set to @c false
    /// if the panel is in read-only mode but the option is not dynamic. In
    /// all other cases, the property will be set to @c true.
    /// @param optionsNodes XML document.
    /// @param m_options Corresponding m_options components.
    void setEnabled(const QDomDocument & optionsNodes,
                    const QList<GraphicalOption *> & options);

    /// @brief Build containers with modified m_options.

    /// This method allows to get old and new values of each modified option.
    /// The old value is the original one, that the option had on calling
    /// @c setOptions. The new value is the current option value. All
    /// intermediate values (i.e. : if user modified several times the same
    /// option) are ignored. These values are stored in @c oldValues and
    /// @c newValues respectively, if they are provided. Each modified
    /// option name is stored in the provides string list. Hash map keys have
    /// one of these names. @n @n
    /// The method garantees that:
    /// @li string list and hash map will have exactly the same number of
    /// elements
    /// @li all hash map keys can be found in the string list
    /// @li each string list item has a corresponding key in both hash maps.
    /// @param graphicalOptions Graphical components corresponding the option
    /// nodes.
    /// @param m_options String list where modified option names will be stored.
    /// @param oldValues If not @c CFNULL, this hash map is used to store old
    /// value of an option. If @c CFNULL, this parameter is not used. The key
    /// is the option name as stored in @c options string list. The value is
    /// the old value.
    /// @param newValues If not @c CFNULL, this hash map is used to store new
    /// value of an option. If @c CFNULL, this parameter is not used. The key
    /// is the option name as stored in @c options string list. The value is
    /// the new value.
    void getModifiedOptions(const QList<GraphicalOption *> & graphicalOptions,
                            CommitDetails & commitDetails,
                            bool newOptions) const;

    /// @brief Checks if options has been modified.

    /// @param graphicalOptions Options to check
    /// @return Returns @c true if at least one option has been modified;
    /// otherwise, returns @c false.
    bool isModified(const QList<GraphicalOption *> & graphicalOptions) const;

    void setOptions(const QList<NodeOption> & list);

    void buttonsSetVisible(bool visible);

  }; // class OptionPanel

  /////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_OptionPanel_h
