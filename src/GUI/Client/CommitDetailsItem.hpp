#ifndef CF_GUI_Client_CommiDetailsItem_h
#define CF_GUI_Client_CommiDetailsItem_h

////////////////////////////////////////////////////////////////////////////////

class QString;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////////

  /// @brief Basic item used by CommitDetails class.

  class CommitDetailsItem
  {

  public:

    /// @brief Constructor

    /// @param optionName Option name.
    /// @param oldValue Old value. May be empty.
    /// @param currentValue Current value. May be empty.
    CommitDetailsItem(const QString & optionName, const QString & oldValue,
                      const QString & currentValue);

    /// @brief Gives the current value.
    /// @return Returns the current value.
    QString getCurrentValue() const;

    /// @brief Give the old value.
    /// @return Returns the old value
    QString getOldValue() const;

    /// @brief Gives the option name
    /// @return Returns the option name
    QString getOptionName() const;

  private:

    /// @brief Option name
    QString m_optionName;

    /// @brief Old value
    QString m_oldValue;

    /// @brief Current value
    QString m_currentValue;


  }; // class CommitDetailsItem

  ////////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_CommiDetailsItem_h
