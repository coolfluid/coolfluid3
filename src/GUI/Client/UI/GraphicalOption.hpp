// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_GraphicalOption_h
#define CF_GUI_Client_GraphicalOption_h

////////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/Core/OptionType.hpp"

class QFormLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QWidget;
class QString;
class QVariant;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

////////////////////////////////////////////////////////////////////////////////

  class GraphicalValue;

  /// @brief Displays an option graphically.

  /// The value component is adapted to the type of the option.

  class GraphicalOption : public QObject
  {
    Q_OBJECT

  public:

    /// @brief Constructor.

    /// @param type Option type. Must be one of those defined by
    /// <code>OptionType::Type</code> enum.
    GraphicalOption(OptionType::Type type, QWidget * parent = CFNULL);

    /// @brief Destructor.

    /// Frees all allocated memory.
    ~GraphicalOption();

    /// @brief Gives the option name.

    /// @return Returns the option name.
    QString getName() const;

    /// @brief Sets option name.

    /// @param name Option name.
    void setName(const QString & name);

    /// @brief Gives the option value.

    /// @return Returns the option value.
    /// @note If the type of the option is @c OptionType::TYPE_FILES,
    /// the variant object returned is a @c QStringList.
    QVariant getValue() const;

    /// @brief Gives the option value in string format.

    /// This method calls @c getValue() and converts the returned variant
    /// object to a @c QString.
    /// @return Returns the value in a string.
    /// @note If the type of the option is @c OptionType::TYPE_FILES,
    /// this method returnes a string with files separated by a white space.
    QString getValueString() const;

    /// @brief Gives option type
    /// @return Returns option type.
    OptionType::Type getType() const;

    /// @brief Adds this option to the provided layout.

    /// @param m_layout Layout to which the m_options has to be added.
    void addToLayout(QFormLayout * layout);

    /// @brief Sets a new value to the option.

    /// @param value New value. Must be in a format compatible with the option
    /// type. Compatible formats list for each type is available in QVariant
    /// class documentation.
    /// @throw CastingFailed If the value could not be converted to
    /// the option type.
    /// @note If the type of the option is @c OptionType::TYPE_FILES or
    /// @c OptionType::TYPE_LIBRAIRIES, the value is a @c QStringList.
    /// Neither files existence nor paths rightness are checked. The list
    /// may be empty.
    void setValue(const QVariant & value);

    /// @brief Enables or disables the value component.

    /// If the component is enabled, its value is modifiable.
    /// @param enabled If @c true, the component is enabled.
    /// Otherwise it is disabled.
    void setEnabled(bool enabled);

    /// @brief Indicates whether the value component is enabled or not.

    /// @return Returns @c true if the component is enabled.
    bool isEnabled() const;

    /// @brief Sets a tooltip.

    /// @param toolTip Tool tip to set.
    void setToolTip(const QString & toolTip);

    /// @brief Indicates whether the value has been modified.

    /// The value is considered to have been modified if it is different from
    /// the last value assigned using @c setValue.
    /// @return Returns @c true if the value has been modified.
    bool isModified() const;

    /// @brief Gives the option original value.

    /// The original value is the last one set using @c setValue.
    /// @return Returns the option original value.
    /// @note If the type of the option is @c OptionTypes::TYPE_FILES,
    /// the variant object returned is a @c QStringList.
    QVariant getOrginalValue() const;

    /// @brief Gives the option value in string format.

    /// This method calls @c getOrginalValue() and converts the returned
    /// variant object to a @c QString.
    /// @return Returns the value in a string.
    /// @note If the type of the option is @c OptionType::TYPE_FILES or
    /// @c OptionType::TYPE_LIBRAIRIES, this method returnss a string with
    /// files separated by a white space.
    QString getOrginalValueString() const;

    /// @brief Commits changes.

    /// Calling this method will set the current option value, given by
    /// @c #getValue, as original value.
    void commit();

  signals:

    void valueChanged();

  private:

    /// @brief Label for the option name.
    QLabel * m_name;

    /// @brief Line edit for the option value.
    GraphicalValue * m_valueWidget;

    /// @brief Type of the option, according to the type ids defined by
    /// OptionTypes class.
    OptionType::Type m_type;

    /// @brief Indicates wether the value component is enabled (allows
    /// modification) or not.
    bool m_enabled;

  }; // class GraphicalOption

  /////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_GraphicalOption_h
