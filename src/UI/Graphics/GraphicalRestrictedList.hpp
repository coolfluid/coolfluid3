// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Graphics_GaphicalRestrictedList_hpp
#define CF_GUI_Graphics_GaphicalRestrictedList_hpp

///////////////////////////////////////////////////////////////////////////////

#include "UI/Graphics/GraphicalValue.hpp"

class QComboBox;

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

  /////////////////////////////////////////////////////////////////////////////

  class Graphics_API GraphicalRestrictedList : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalRestrictedList(CF::Common::Option::ConstPtr opt = CF::Common::Option::ConstPtr(),
                            QWidget * parent = 0);

    ~GraphicalRestrictedList();

    void setRestrictedList(const QStringList & list);

    virtual bool setValue(const QVariant & value);

    virtual QVariant value() const;

  private slots:

    void currentIndexChanged(int);

  private:

    QComboBox * m_comboChoices;

    template<typename TYPE>
    void vectToStringList(const std::vector<boost::any> & vect,
                          QStringList & list) const;
  }; // class GraphicalRestrictedList

  /////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Graphics_GaphicalRestrictedList_hpp
