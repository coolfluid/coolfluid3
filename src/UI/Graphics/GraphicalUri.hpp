// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Graphics_GraphicalUrl_hpp
#define CF_GUI_Graphics_GraphicalUrl_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/OptionURI.hpp"

#include "UI/Graphics/GraphicalValue.hpp"

class QComboBox;
class QCompleter;
class QLineEdit;
class QPushButton;
class QString;
class QStringListModel;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////

  class Graphics_API GraphicalUri : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUri(CF::Common::OptionURI::ConstPtr opt = CF::Common::OptionURI::ConstPtr(),
                 QWidget *parent = 0);

    ~GraphicalUri();

    virtual bool setValue(const QVariant & path);

    virtual QVariant value() const;

    void setSchemes(const std::vector<Common::URI::Scheme::Type> & list);

  private slots:

    void btBrowseClicked();

    void updateModel(const QString & path);

    void changeType(const QString & type);

  private:

    QLineEdit * m_editPath;

    QPushButton * m_btBrowse;

    QComboBox * m_comboType;

    QCompleter * m_completer;

    QStringListModel * m_completerModel;

    QString m_currentType;

  }; // class GraphicalUrl

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Graphics_GraphicalUrl_hpp
