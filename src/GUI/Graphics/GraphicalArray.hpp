// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Graphics_GraphicalArray_hpp
#define CF_GUI_Graphics_GraphicalArray_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Graphics/GraphicalValue.hpp"

class QGroupBox;
class QGridLayout;
class QKeyEvent;
class QLineEdit;
class QListView;
class QPushButton;
class QStringListModel;
class QValidator;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Graphics {

  //////////////////////////////////////////////////////////////////////////


  class GraphicalArray : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalArray(QValidator * validator = nullptr, QWidget * parent = nullptr);

    ~GraphicalArray();

    void setValidator(QValidator * validator);

    virtual bool setValue(const QVariant & path);

    virtual QVariant value() const;

  protected:

    void keyPressEvent(QKeyEvent * event);

  private slots:

    void btRemoveClicked();

  private:

    QLineEdit * m_editAdd;

    QStringListModel * m_model;

    QListView * m_listView;

    QPushButton * m_btRemove;

    QGridLayout * m_boxLayout;

    QGroupBox * m_groupBox;

  }; // class GraphicalArray

  //////////////////////////////////////////////////////////////////////////

} // Graphics
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Graphics_GraphicalArray_hpp
