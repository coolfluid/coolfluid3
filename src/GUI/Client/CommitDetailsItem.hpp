#ifndef CF_GUI_Client_CommiDetailsItem_h
#define CF_GUI_Client_CommiDetailsItem_h

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {
    
/////////////////////////////////////////////////////////////////////////////
    
    class CommitDetailsItem 
    {
      
    public:
      
      CommitDetailsItem(const QString & optionName, const QString & oldValue, 
                        const QString & currentValue);
      
      CommitDetailsItem(const QString & oldValue, const QString & currentValue);
      
      bool isNewOption() const;
      
      QString getCurrentValue() const;
      
      QString getOldValue() const;
      
      QString getOptionName() const;
      
    private:
      
      QString m_optionName;
      
      QString m_oldValue;
      
      QString m_currentValue;
      
      bool m_newOption;
      
      
    }; // class CommitDetailsItem
    
////////////////////////////////////////////////////////////////////////////
    
} // namespace Client
} // namespace GUI 
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_CommiDetailsItem_h