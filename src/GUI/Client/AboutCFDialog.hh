#ifndef CF_GUI_Client_AboutCFDialog_h
#define CF_GUI_Client_AboutCFDialo.hh

//////////////////////////////////////////////////////////////////////////////

#include <QDialog>
#include <QList>

class QFormLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;
  
/////////////////////////////////////////////////////////////////////////////
  
namespace CF {
namespace GUI {
namespace Client {

///////////////////////////////////////////////////////////////////////////////

  class AboutCFDialog : public QDialog
  {
    struct CFInfo
    {
      public:
      QLabel * labName;
      QLabel * labValue;
      
      CFInfo(const QString & name, const QString & value, QFormLayout * parent);
      
      ~CFInfo();
    };
    
   public:
    
    AboutCFDialog(QWidget * parent = NULL);
    
    ~AboutCFDialog();
    
   private: // data
    
    QVBoxLayout * m_mainLayout;
    
    QPushButton * m_btOK;
    
    QFormLayout * m_infoLayout;
    
    QList<CFInfo *> m_infoList;
    
  }; // class AboutCFDialog
  
  ///////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI   
} // namespace CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_AboutCFDialo.hh
