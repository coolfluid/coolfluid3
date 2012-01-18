// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>

#include "ui/core/CommitDetails.hpp"

#include "ui/graphics/ModifiedOptionsDialog.hpp"

using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

///////////////////////////////////////////////////////////////////////////

ModifiedOptionsDialog::ModifiedOptionsDialog(QWidget * parent)
: QDialog(parent)
{
  //CommitDetails details;
  m_main_layout = new QVBoxLayout(this);
  m_button_box = new QDialogButtonBox(this);
  m_view = new QTableView(this);

  m_button_box->addButton(QDialogButtonBox::Ok);

  this->resize(this->width(), this->height());

  m_main_layout->addWidget(m_view, 0);
  m_main_layout->addWidget(m_button_box, 1);

  connect(m_button_box, SIGNAL(accepted()), this, SLOT(close()));
}

///////////////////////////////////////////////////////////////////////////

ModifiedOptionsDialog::~ModifiedOptionsDialog()
{
 delete m_main_layout;
 delete m_button_box;
 delete m_view;
}

///////////////////////////////////////////////////////////////////////////

void ModifiedOptionsDialog::show(CommitDetails & details)
{
  if (details.rowCount() > 0)
  {
    m_view->setModel(&details);

    m_view->resizeColumnsToContents();

    this->exec();
  }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
