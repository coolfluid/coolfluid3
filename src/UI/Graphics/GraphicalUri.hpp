// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_GraphicalUrl_hpp
#define cf3_GUI_Graphics_GraphicalUrl_hpp

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

namespace cf3 {
namespace UI {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////

  class Graphics_API GraphicalUri : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUri(cf3::common::OptionURI::ConstPtr opt = cf3::common::OptionURI::ConstPtr(),
                 QWidget *parent = 0);

    ~GraphicalUri();

    virtual bool setValue(const QVariant & path);

    virtual QVariant value() const;

    void setSchemes(const std::vector<common::URI::Scheme::Type> & list);

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
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Graphics_GraphicalUrl_hpp
