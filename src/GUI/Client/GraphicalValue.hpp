#ifndef CF_GUI_Client_GraphicalValue_hpp
#define CF_GUI_Client_GraphicalValue_hpp

////////////////////////////////////////////////////////////////////////////

#include <QVariant>
#include <QWidget>

class QHBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class GraphicalValue : public QWidget
  {
  public:

    GraphicalValue(QWidget * parent = 0);

    virtual bool setValue(const QVariant & value) = 0;

    virtual QVariant getValue() const = 0;

    QString getValueString() const;

    QVariant getOriginalValue() const;

    QString getOriginalString() const;

    bool isModified() const;

    void commit();

  protected:

    QVariant m_originalValue;

    QHBoxLayout * m_layout;

    bool m_committing;

  }; // class GraphicalValue

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_GraphicalValut_hpp
