// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Client_UI_TabBuilder_hpp
#define cf3_GUI_Client_UI_TabBuilder_hpp

///////////////////////////////////////////////////////////////////////////

#include <QList>
#include <QMap>
#include <QTabWidget>

#include "UI/Core/CNode.hpp"

#include "UI/Graphics/LibGraphics.hpp"

///////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

///////////////////////////////////////////////////////////////////////////

struct TabInfo
{
  QWidget * widget;

  int tabIndex;

  bool isVisible;
};

class TabBuilder : public QTabWidget
{
  Q_OBJECT


public:

  static TabBuilder * instance();

  template<typename TYPE>
  TYPE * getWidget( Core::CNode::ConstPtr node )
  {
    TYPE * widget = nullptr;
    std::string key = node->properties().value_str("uuid"); //node->uri().path();

    if( !m_tabs.contains(key) )
    {
      TabInfo info;

      info.widget = new TYPE(/*this*/);
      info.tabIndex = addTab(info.widget, node->uri().path().c_str());
      info.isVisible = true;
      m_tabs[key] = info;
    }
    else
    {
      setTabText( m_tabs[key].tabIndex, node->uri().path().c_str() );
      m_newTabs.append( key );
    }

    widget = static_cast<TYPE*>(m_tabs[key].widget);

    cf3_assert ( is_not_null(widget) );
    return widget;
  }

  void showTab( Core::CNode::ConstPtr node );

  void queueTab( Core::CNode::ConstPtr node);

private slots:

  void tabClicked(int index);

  void beginModelReset();

  void endModelReset();

private: // functions

  TabBuilder(QWidget * parent = 0);

  ~TabBuilder();

private : // data

  QMap<std::string, TabInfo> m_tabs;

  QMap<std::string, int> m_lastTabs;

  QList<std::string> m_newTabs;

}; // TabManager

///////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

///////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Client_UI_TabBuilder_hpp
