// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_OptionPanel_h
#define cf3_GUI_Graphics_OptionPanel_h

////////////////////////////////////////////////////////////////////////////////

#include <QLabel>
#include <QMap>
#include <QFormLayout>

#include "common/Option.hpp"

#include "UI/Graphics/LibGraphics.hpp"

class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QModelIndex;
class QPushButton;
class QScrollArea;
class QSplitter;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {

namespace Core { class CommitDetails; }

namespace Graphics {

////////////////////////////////////////////////////////////////////////////////

  class GraphicalValue;
  struct CloseConfirmationInfos;

  /// @brief Panel to m_view and modify m_options of an object.

  /// This class allows user to display and modify options of an object.

  /// @author Quentin Gasper.

  class Graphics_API OptionLayout : public QFormLayout
  {
    Q_OBJECT

  public:
    /// @brief Constructor.

    /// Builds an @c OptionPanel with no options. The panel is neither in
    /// read-only mode nor advanced mode.
    /// @param parent The parent widget. Default value is @c nullptr
    OptionLayout(QWidget * parent = nullptr);

    /// @brief Destructor.

    /// Frees the allocated memory.  Parent is not destroyed.
    ~OptionLayout();

    /// @brief Destroys all graphical values.
    void clearOptions();

    /// @brief Indicates wether at least on option has been modified.

    /// @return Returns @c true if at least one option has been modified.
    bool isModified() const;

    /// @brief Gathers modified options.

    /// @param commitDetails Object where modified values will be stored. The
    /// object is not cleared.
    void modifiedOptions(Core::CommitDetails & commitDetails) const;

    void addOption(cf3::common::Option::ConstPtr option);

    bool hasOptions() const;

    /// @brief Puts options in a provided map.

    /// @param options A hashmap were modified options will be written. The
    /// key is the option name and the value is the option new value.
    /// @param all If @c false, only modified options will be put, meaning
    /// that the hashmap may be empty. If @c true, all option are put.
    void options(QMap<QString, QString> & options, bool all) const;

    /// @brief Marks options as commited

    /// Calls GraphicalOption::commit() for each graphical option the layout
    /// contains.
    void commitOpions();

  signals:

    void valueChanged();

  private:

    /// @brief List containing basic m_options components.
    QMap<QString, GraphicalValue *> m_options;

     /// @brief Indicates if the panel is in advanced mode or not.

    /// If @c true, the panel is in advanced mode. Advanced m_options (if any)
    /// are displayed. Otherwise, they are m_hidden.
    bool m_advancedMode;

    void setOptions(const QList<cf3::common::Option::ConstPtr> & list);

  }; // class OptionPanel

  /////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Graphics_OptionPanel_h
