// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_GaphicalRestrictedList_hpp
#define cf3_ui_Graphics_GaphicalRestrictedList_hpp

///////////////////////////////////////////////////////////////////////////////

#include "ui/graphics/GraphicalValue.hpp"

class QComboBox;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

  /////////////////////////////////////////////////////////////////////////////

  class Graphics_API GraphicalRestrictedList : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalRestrictedList(cf3::common::Option::ConstPtr opt = cf3::common::Option::ConstPtr(),
                            QWidget * parent = 0);

    ~GraphicalRestrictedList();

    void set_restricted_list(const QStringList & list);

    virtual bool set_value(const QVariant & value);

    virtual QVariant value() const;

  private slots:

    void current_index_changed(int);

  private:

    QComboBox * m_comboChoices;

    template<typename TYPE>
    void vect_to_stringlist(const std::vector<boost::any> & vect,
                          QStringList & list) const;
  }; // class GraphicalRestrictedList

  /////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

///////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_GaphicalRestrictedList_hpp
