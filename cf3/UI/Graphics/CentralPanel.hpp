// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_CentralPanel_h
#define cf3_GUI_Graphics_CentralPanel_h

////////////////////////////////////////////////////////////////////////////////

#include <QWidget>

#include "common/Option.hpp"

#include "UI/Graphics/LibGraphics.hpp"

class QGridLayout;
class QGroupBox;
class QModelIndex;
class QPushButton;
class QScrollArea;
class QSplitter;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {

namespace Core { class CommitDetails; }

namespace Graphics {

////////////////////////////////////////////////////////////////////////////////

  class GraphicalValue;
  class OptionLayout;
  struct CloseConfirmationInfos;

  /// @brief Panel to view and modify options of an object.

  /// This class allows user to display and modify options of an object.

  /// @author Quentin Gasper.

  class Graphics_API CentralPanel : public QWidget
  {
    Q_OBJECT

  public:

    /// Constructor.

    /// Builds an @c CentralPanel with no options. The panel is neither in
    /// read-only mode nor advanced mode.
    /// @param parent The parent widget. Default value is @c nullptr
    CentralPanel(QWidget * parent = nullptr);

    /// Destructor.

    /// Frees the allocated memory.  Parent is not destroyed.
    ~CentralPanel();

    /// Indicates wether at least one option has been modified.

    /// @return Returns @c true if at least one option has been modified.
    bool isModified() const;

    /// Build containers with modified options.

    /// This method allows to get old and new values of each modified option.
    /// The old value is the original one, that the option had when it was
    /// created. The new value is the current option value. All intermediate
    /// values (i.e. : if user modified several times the same option) are
    /// ignored.
    /// @param commitDetails The object where values will be stored.
    void modifiedOptions(Core::CommitDetails & commitDetails) const;

    /// Gives the current path.

    /// @return Returns the current path.
    QString currentPath() const;

  public slots:

    /// Slot called when user clicks on "Commit changes" button.

    /// If at least one option has been modified, @c changesMade signal is
    /// emitted.
    void btApplyClicked();

  private slots:

    /// Slot called when the model current index changed.

    /// Options are replaced by the new ones.
    /// @param newIndex The new current index.
    /// @param oldIndex The old current index. This parameter is not used.
    void current_index_changed(const QModelIndex & newIndex, const QModelIndex & oldIndex);

    /// Slot called when the model advanced status has changed.

    /// If status is advanced, the basic options layout is always visible,
    /// so that the user knows he is playing with advanced options. If
    /// the current option has no advanced option, the advanced option
    /// layout remains hidden.
    /// @param advanced If @c true, advanced options are showed up.
    void advanced_mode_changed(bool advanced);

    /// Slot called when data changed in the underlying model.

    /// This slot considers that only one index has changed, instead of a range.
    /// If the @c first, the @c last and the model current indexes are the
    /// same and valid, the option layouts are updated with the new options.
    /// @param first The first index of the range.
    /// @param last The last index of the range.
    void dataChanged(const QModelIndex & first, const QModelIndex & last);

    /// Slot called when user wants to see what options have been modified.
    void btSeeChangesClicked();

    /// Slot called when user wants to clear all the changes.
    void btForgetClicked();

    /// Slot called when an option value has been modified.
    void valueChanged();

  private:

    /// Scroll area for basic options
    QScrollArea * m_scrollBasicOptions;

    /// Scroll area for advanced options
    QScrollArea * m_scrollAdvancedOptions;

    /// List containing basic options components.
    OptionLayout * m_basicOptionLayout;

    /// List containing advanced options components.
    OptionLayout * m_advancedOptionLayout;

    /// Button used to commit changes made.
    QPushButton * m_btApply;

    /// Button used to reset changes.
    QPushButton * m_btForget;

    /// Button used to see changes made.
    QPushButton * m_btSeeChanges;

    /// Main layout containing basic and advanced option panels.

    /// This layout is composed of three lines and one column.
    QGridLayout * m_mainLayout;

    /// Groupbox used to display basic m_options components
    /// with a titled border.

    ///  Its layout is @c #m_basicOptionsLayout.
    QGroupBox * m_gbBasicOptions;

    /// Groupbox used to display advanced m_options components
    /// with a titled border.

    ///  Its layout is @c #m_advancedOptionsLayout.
    QGroupBox * m_gbAdvancedOptions;

    /// Indicates if the panel is in advanced mode or not.

    /// If @c true, the panel is in advanced mode. Advanced m_options (if any)
    /// are displayed. Otherwise, they are m_hidden.
    bool m_advancedMode;

    /// The path of the component of which options are currently displayed.
    QString m_currentPath;

    /// Splitter between basic and advanced options.
    QSplitter * m_splitter;

    /// Sets new graphical options.

    /// All existing options in the layouts are removed.
    /// @param list The list of options to set. Can be empty.
    /// @todo why don't we use PropertyList?
    void setOptions(const QList<cf3::common::Option::ConstPtr> & list);

    /// Set the buttons to a visible/invisible state.

    /// @param visible If @c true, buttons are set to visible.
    void setButtonsVisible(bool visible);

    /// Set the buttons to a enabled/disabled state.

    /// @param enabled If @c true, buttons are set to enabled.
    void setButtonsEnabled(bool enabled);

  }; // CentralPanel

  /////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Graphics_CentralPanel_h
