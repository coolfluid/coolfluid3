// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Graphics_CentralPanel_h
#define CF_GUI_Graphics_CentralPanel_h

////////////////////////////////////////////////////////////////////////////////

#include <QWidget>

#include "Common/Option.hpp"

#include "GUI/Graphics/LibGraphics.hpp"

class QGridLayout;
class QGroupBox;
class QModelIndex;
class QPushButton;
class QScrollArea;
class QSplitter;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

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

    /// @brief Constructor.

    /// Builds an @c CentralPanel with no options. The panel is neither in
    /// read-only mode nor advanced mode.
    /// @param parent The parent widget. Default value is @c nullptr
    CentralPanel(QWidget * parent = nullptr);

    /// @brief Destructor.

    /// Frees the allocated memory.  Parent is not destroyed.
    ~CentralPanel();

    /// @brief Indicates wether at least one option has been modified.

    /// @return Returns @c true if at least one option has been modified.
    bool isModified() const;

    /// @brief Build containers with modified options.

    /// This method allows to get old and new values of each modified option.
    /// The old value is the original one, that the option had when it was
    /// created. The new value is the current option value. All intermediate
    /// values (i.e. : if user modified several times the same option) are
    /// ignored.
    /// @param commitDetails The object where values will be stored.
    void modifiedOptions(Core::CommitDetails & commitDetails) const;

    /// @brief Gives the current path.

    /// @return Returns the current path.
    QString currentPath() const;

  public slots:

    /// @brief Slot called when user clicks on "Commit changes" button.

    /// If at least one option has been modified, @c changesMade signal is
    /// emitted.
    void btApplyClicked();

  private slots:

    /// Slot called when the model current index changed.

    /// Options are replaced by the new ones.
    /// @param newIndex The new current index.
    /// @param oldIndex The old current index. This parameter is not used.
    void currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex);

    /// Slot called when the model advanced status has changed.

    /// If status is advanced, the basic options layout is always visible,
    /// so that the user knows he is playing with advanced options. If
    /// the current option has no advanced option, the advanced option
    /// layout remains hidden.
    /// @param advanced If @c true, advanced options are showed up.
    void advancedModeChanged(bool advanced);

    /// Slot called when data changed in the underlying model.

    ///
    ///
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

} // Graphics
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Graphics_CentralPanel_h
