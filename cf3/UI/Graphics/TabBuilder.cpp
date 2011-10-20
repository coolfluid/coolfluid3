// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/NTree.hpp"

#include "UI/Graphics/TabBuilder.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;

///////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

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
  connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabClicked(int)));

  connect( NTree::globalTree().get(), SIGNAL(beginUpdateTree()),
           this, SLOT(beginModelReset()) );

  connect( NTree::globalTree().get(), SIGNAL(endUpdateTree()),
           this, SLOT(endModelReset()) );

}

///////////////////////////////////////////////////////////////////////////

TabBuilder::~TabBuilder()
{

}

///////////////////////////////////////////////////////////////////////////

void TabBuilder::beginModelReset()
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

void TabBuilder::endModelReset()
{
  QMap<std::string, int>::iterator it = m_lastTabs.begin();

  while( it != m_lastTabs.end() )
  {
    m_tabs.remove( it.key() );
    removeTab( it.value() );
    it++;
  }

  m_lastTabs.clear();

  QMap<std::string, TabInfo>::iterator itTabs = m_tabs.begin();

  while( itTabs != m_tabs.end() )
  {
    itTabs.value().tabIndex = indexOf( itTabs.value().widget );
    itTabs++;
  }


}

///////////////////////////////////////////////////////////////////////////

void TabBuilder::showTab( CNode::ConstPtr node )
{
  std::string key = node->properties().value_str("uuid"); //node->uri().path();

  if( m_tabs.contains(key) )
    setCurrentIndex( m_tabs[key].tabIndex );
  else
    throw ValueNotFound(FromHere(), "No tab for component [" +
                        node->uri().path() + "] was found.");
}

//////////////////////////////////////////////////////////////////////////////

void TabBuilder::queueTab(Core::CNode::ConstPtr node)
{
  std::string uuid = node->properties().value_str("uuid");

  if( m_tabs.contains( uuid ) )
  {
    m_lastTabs[uuid] = m_tabs[uuid].tabIndex;
  }
}

///////////////////////////////////////////////////////////////////////////

void TabBuilder::tabClicked(int index)
{
  if(index > 0)
  {
    QModelIndex newIndex = NTree::globalTree()->indexFromPath(tabText(index).toStdString());

    if( newIndex.isValid() )
      NTree::globalTree()->setCurrentIndex( newIndex );
  }
}

///////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // cf3
