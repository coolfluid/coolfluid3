#ifndef CF_GUI_Client_ShutdownConfirmationPanel_h
#define CF_GUI_Client_ShutdownConfirmationPanel_h

//////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/CloseConfirmationPanel.hh"

class QComboBox;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {
   
/////////////////////////////////////////////////////////////////////////////
    
  struct CloseConfirmationInfos;
  
/////////////////////////////////////////////////////////////////////////////
  
  class ShutdownConfirmationPanel : public CloseConfirmationPanel
  {
    
    public:
    
    ShutdownConfirmationPanel(QDialog * parent = NULL);
    
    ~ShutdownConfirmationPanel();
    
    virtual bool isAccepted() const;
    
    virtual void getData(CloseConfirmationInfos & infos) const;
    
    virtual void setData(const CloseConfirmationInfos & infos);
    
    protected:
    
    virtual void hideComponents(bool hide);
    
    private:
    
    QComboBox * m_comboBox;
    
  }; // class ShutdownConfirmationPanel
  
/////////////////////////////////////////////////////////////////////////////
  
} // namespace Client
} // namespace GUI 
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ShutdownConfirmationPanel_h
