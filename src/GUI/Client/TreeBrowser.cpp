#include <QGridLayout>
#include <QMenu>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <QDebug>

#include "Common/CPath.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/TreeView.hpp"

#include "GUI/Client/TreeBrowser.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

TreeBrowser::TreeBrowser(TreeView * view, QWidget *parent) :
    QWidget(parent),
    m_treeView(view),
    m_currentIndex(0)
{
  cf_assert(view != CFNULL);

  m_btPrevious = new QToolButton(this);
  m_btNext = new QToolButton(this);
  m_menuNext = new QMenu(m_btNext);
  m_menuPrevious = new QMenu(m_btPrevious);

  m_buttonsLayout = new QGridLayout();
  m_mainLayout = new QVBoxLayout(this);

  m_btPrevious->setArrowType(Qt::LeftArrow);
  m_btNext->setArrowType(Qt::RightArrow);

  m_btNext->setMenu(m_menuNext);
  m_btPrevious->setMenu(m_menuPrevious);

  m_btNext->setPopupMode(QToolButton::MenuButtonPopup);
  m_btPrevious->setPopupMode(QToolButton::MenuButtonPopup);

  m_buttonsLayout->addWidget(m_btPrevious, 0, 0);
  m_buttonsLayout->addWidget(m_btNext, 0, 1);
  m_buttonsLayout->addWidget(new QWidget(), 0, 2);

  m_buttonsLayout->setColumnStretch(2, 10);

  m_mainLayout->addLayout(m_buttonsLayout);
  m_mainLayout->addWidget(m_treeView);

  connect(m_btPrevious, SIGNAL(clicked()), this, SLOT(previousClicked()));
  connect(m_btNext, SIGNAL(clicked()), this, SLOT(nextClicked()));

  connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(doubleClicked(QModelIndex)));

  m_history << m_treeView->rootIndex();
  this->updateButtons();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TreeBrowser::~TreeBrowser()
{
  delete m_menuNext;
  delete m_menuPrevious;
  delete m_buttonsLayout;
  delete m_mainLayout;
  delete m_btNext;
  delete m_btPrevious;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeBrowser::previousClicked()
{
  cf_assert(m_currentIndex > 0);

  m_currentIndex--;

  const QModelIndex & index = m_history.at(m_currentIndex);

//  if(!index.parent().isValid())
//    m_treeView->setRootIndex(index.parent());
//  else
    m_treeView->setRootIndex(index.parent());

  this->updateButtons();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeBrowser::nextClicked()
{
  cf_assert(m_currentIndex < m_history.size() - 1);

  m_currentIndex++;
  m_treeView->setRootIndex(m_history.at(m_currentIndex));
  this->updateButtons();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeBrowser::doubleClicked(const QModelIndex & index)
{
  while(m_currentIndex < m_history.size() - 1)
    m_history.removeLast();

  m_history << QPersistentModelIndex(index);
  //m_actions << QAction(m_treeView->getPath(index).string().c_string());
  m_currentIndex++;
  m_treeView->setRootIndex(index);
  this->updateButtons();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeBrowser::actionTriggered()
{
  QAction * action = static_cast<QAction*>(sender());

  if(action != CFNULL && m_actions.contains(action))
  {
    m_currentIndex = m_actions[action];
    m_treeView->setRootIndex(m_history.at(m_currentIndex));
    this->updateButtons();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TreeBrowser::updateButtons()
{
  m_actions.clear();
  m_menuNext->clear();
  m_menuPrevious->clear();

  if(!m_history.isEmpty())
  {
    for(int i = 0 ; i < m_history.size() ; i++)
    {
      QPersistentModelIndex index = m_history.at(i);
      QString path = m_treeView->getPath(index).string().c_str();
      QIcon icon = m_treeView->getIcon(index);

      if(path.isEmpty())
      {
        path = ClientRoot::tree()->getRoot()->root()->full_path().string().c_str();
        icon = ClientRoot::tree()->getRoot()->getIcon();
      }

      QAction * action = new QAction(icon, path, this);

      connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
      m_actions[action] = i;

        if(i < m_currentIndex)
          m_menuPrevious->addAction(action);
        else if(i > m_currentIndex)
          m_menuNext->addAction(action);
    }
  }

  m_btNext->setEnabled(!m_menuNext->isEmpty());
  m_btPrevious->setEnabled(!m_menuPrevious->isEmpty());
}
