// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Graphics_GraphicalUriArray_hpp
#define CF_GUI_Graphics_GraphicalUriArray_hpp

////////////////////////////////////////////////////////////////////////////

#include "UI/Graphics/GraphicalValue.hpp"

class QComboBox;
class QKeyEvent;
class QGridLayout;
class QGroupBox;
class QItemSelection;
class QLineEdit;
class QListView;
class QPushButton;
class QStringListModel;
class QValidator;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

  //////////////////////////////////////////////////////////////////////////


  class GraphicalUriArray : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUriArray(const QString & sep = QString(), QWidget * parent = nullptr);

    ~GraphicalUriArray();

    void setProtocols(const std::vector<std::string> & protocols);

    virtual bool setValue(const QVariant & path);

    virtual QVariant value() const;

  private slots:

    void btAddClicked();

    void btRemoveClicked();

    void changeType(const QString & type);

    void moveUp();

    void moveDown();

    void selectionChanged(const QItemSelection& selected, const QItemSelection & delected);

  private: // functions

    void moveItems( int step );

  private:

    QLineEdit * m_editAdd;

    QStringListModel * m_model;

    QListView * m_listView;

    QPushButton * m_btAdd;

    QPushButton * m_btRemove;

    QPushButton * m_btUp;

    QPushButton * m_btDown;

    QVBoxLayout * m_buttonsLayout;

    QComboBox * m_comboType;

    QGridLayout * m_boxLayout;

    QGroupBox * m_groupBox;

  }; // class GraphicalArray

  //////////////////////////////////////////////////////////////////////////

} // namespace Graphics
} // namespace UI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Graphics_GraphicalUriArray_hpp
