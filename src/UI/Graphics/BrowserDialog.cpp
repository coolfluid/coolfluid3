// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QTableView>

#include "UI/Core/NRemoteFSBrowser.hpp"
#include "UI/Core/ThreadManager.hpp"
#include "UI/Core/TreeThread.hpp"

#include "UI/Graphics/BrowserDialog.hpp"

using namespace CF::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////

BrowserDialog::BrowserDialog(QWidget *parent) :
    QDialog(parent)
{
  m_model = NRemoteFSBrowser::Ptr(new NRemoteFSBrowser("MyBrowser"));
  m_view = new QTableView( this );
  m_favoritesView = new QListView( this );
  m_labPath = new QLabel("Path:");
  m_labFilter = new QLabel("Filter:");
  m_editFilter = new QLineEdit();
  m_editPath = new QLineEdit();
  m_comboFilter = new QComboBox();
  m_btAddFav = new QPushButton("Add");
  m_btRemoveFav = new QPushButton("Remove");
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  m_pathLayout = new QHBoxLayout();
  m_favButtonsLayout = new QHBoxLayout();
  m_filterLayout = new QHBoxLayout();

  m_mainLayout = new QGridLayout(this);

  m_view->setModel( m_model.get() );

  m_comboFilter->addItems( QStringList() << "Wildcards" << "Regular Expression");

  m_mainLayout->setSpacing(5);

  m_view->setSortingEnabled(true);
  m_view->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_view->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_view->setAlternatingRowColors(true);
  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_view->horizontalHeader()->setStretchLastSection(true);

  m_labPath->setBuddy(m_editPath);
  m_labFilter->setBuddy(m_editFilter);

  m_pathLayout->addWidget(m_labPath);
  m_pathLayout->addWidget(m_editPath);

  m_favButtonsLayout->addWidget(m_btAddFav);
  m_favButtonsLayout->addWidget(m_btRemoveFav);

  m_filterLayout->addWidget(m_labFilter);
  m_filterLayout->addWidget(m_editFilter);
  m_filterLayout->addWidget(m_comboFilter);

  m_mainLayout->addLayout(m_pathLayout,       0, 0, 1, -1);
  m_mainLayout->addWidget(m_favoritesView,    1, 0);
  m_mainLayout->addWidget(m_view,             1, 1, 2, -1);
  m_mainLayout->addLayout(m_favButtonsLayout, 2, 0);
  m_mainLayout->addLayout(m_filterLayout,     3, 0, 1, 2);
  m_mainLayout->addWidget(m_buttons,          3, 3);

  m_mainLayout->setColumnStretch(0, 1);
  m_mainLayout->setColumnStretch(1, 2);
  m_mainLayout->setColumnStretch(2, 0);
  m_mainLayout->setColumnStretch(3, 2);

  ThreadManager::instance().tree().root()->addNode(m_model);

  connect(m_comboFilter, SIGNAL(currentIndexChanged(int)),
          this, SLOT(filterTypeChanged(int)));

//  this->resize(QSize(this->width() /** 1.25*/, this->height() /** 1.25*/) );
  m_view->adjustSize();
  this->adjustSize();
  m_model->openDir("");

}

////////////////////////////////////////////////////////////////////////////

void BrowserDialog::filterTypeChanged(int index)
{
  m_view->resizeRowsToContents();
}

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF
