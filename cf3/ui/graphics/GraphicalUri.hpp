// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_GraphicalUrl_hpp
#define cf3_ui_Graphics_GraphicalUrl_hpp

////////////////////////////////////////////////////////////////////////////

#include "common/OptionURI.hpp"

#include "ui/graphics/GraphicalValue.hpp"

class QComboBox;
class QCompleter;
class QLineEdit;
class QPushButton;
class QString;
class QStringListModel;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

////////////////////////////////////////////////////////////////////////////

  class Graphics_API GraphicalUri : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUri(cf3::common::OptionURI::ConstPtr opt = cf3::common::OptionURI::ConstPtr(),
                 QWidget *parent = 0);

    ~GraphicalUri();

    virtual bool set_value(const QVariant & path);

    virtual QVariant value() const;

    void set_schemes(const std::vector<common::URI::Scheme::Type> & list);

  private slots:

    void bt_browse_clicked();

    void update_model(const QString & path);

    void scheme_changed(const QString & type);

  private:

    QLineEdit * m_edit_path;

    QPushButton * m_bt_browse;

    QComboBox * m_combo_schemes;

    QCompleter * m_completer;

    QStringListModel * m_completer_model;

    QString m_current_type;

  }; // class GraphicalUrl

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_GraphicalUrl_hpp
