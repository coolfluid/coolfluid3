// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/NTree.hpp"

#include "UI/Graphics/TabBuilder.hpp"

using namespace CF::Common;
using namespace CF::UI::Core;

///////////////////////////////////////////////////////////////////////////

namespace CF {
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

}

///////////////////////////////////////////////////////////////////////////

TabBuilder::~TabBuilder()
{

}

///////////////////////////////////////////////////////////////////////////

void TabBuilder::showTab( CNode::ConstPtr node )
{
  std::string key = node->uri().path();

  if( m_tabs.contains(key) )
    setCurrentIndex( m_tabs[key].tabIndex );
  else
    throw ValueNotFound(FromHere(), "No tab for component [" +
                        node->uri().path() + "] was found.");
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
} // CF
