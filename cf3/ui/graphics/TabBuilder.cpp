// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ui/core/NTree.hpp"

#include "ui/graphics/TabBuilder.hpp"

using namespace cf3::common;
using namespace cf3::ui::core;

///////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

///////////////////////////////////////////////////////////////////////////

TabBuilder * TabBuilder::instance()
{
  static TabBuilder * inst = new TabBuilder();
  return inst;
}

///////////////////////////////////////////////////////////////////////////

TabBuilder::TabBuilder(QWidget * parent)
  : QTabWidget(parent)
{
  connect(this, SIGNAL(currentChanged(int)), this, SLOT(tab_clicked(int)));

  connect( NTree::global().get(), SIGNAL(begin_update_tree()),
           this, SLOT(begin_model_reset()) );

  connect( NTree::global().get(), SIGNAL(end_update_tree()),
           this, SLOT(end_model_reset()) );

}

///////////////////////////////////////////////////////////////////////////

TabBuilder::~TabBuilder()
{

}

///////////////////////////////////////////////////////////////////////////

void TabBuilder::begin_model_reset()
{
//  QMap<std::string, TabInfo>::iterator it = m_tabs.begin();

//  while( it != m_tabs.end() )
//  {
//    m_lastTabs[it.key()] = it.value().tabIndex;
//    it++;
//  }

//  qDebug() << "last tabs" << m_lastTabs << "in" << __FUNCTION__;
//  qDebug() << "tabs" << m_tabs << "in" << __FUNCTION__;
}

///////////////////////////////////////////////////////////////////////////

void TabBuilder::end_model_reset()
{
  QMap<common::UUCount, int>::iterator it = m_last_tabs.begin();

  while( it != m_last_tabs.end() )
  {
    if( !m_new_tabs.contains(it.key()) )
    {
      m_tabs.remove( it.key() );
      removeTab( it.value() );
    }
    it++;
  }

  m_last_tabs.clear();
  m_new_tabs.clear();

  QMap<common::UUCount, TabInfo>::iterator itTabs = m_tabs.begin();

  while( itTabs != m_tabs.end() )
  {
    itTabs.value().tabIndex = indexOf( itTabs.value().widget );
    itTabs++;
  }
}

///////////////////////////////////////////////////////////////////////////

void TabBuilder::show_tab( CNode::ConstPtr node )
{
  common::UUCount key = node->properties().value<common::UUCount>("uuid"); //node->uri().path();

  if( m_tabs.contains(key) )
    setCurrentIndex( m_tabs[key].tabIndex );
  else
    throw ValueNotFound(FromHere(), "No tab for component [" +
                        node->uri().path() + "] was found.");
}

//////////////////////////////////////////////////////////////////////////////

void TabBuilder::queue_tab(core::CNode::ConstPtr node)
{
  std::string uuid = node->properties().value_str("uuid");

  if( m_tabs.contains( uuid ) )
     m_last_tabs[uuid] = m_tabs[uuid].tabIndex;
}

///////////////////////////////////////////////////////////////////////////

void TabBuilder::tab_clicked(int index)
{
  if(index > 0)
  {
    QModelIndex newIndex = NTree::global()->index_from_path(tabText(index).toStdString());

    if( newIndex.isValid() )
      NTree::global()->set_current_index( newIndex );
  }
}

///////////////////////////////////////////////////////////////////////////

} // Clientui
} // Gui
} // cf3
