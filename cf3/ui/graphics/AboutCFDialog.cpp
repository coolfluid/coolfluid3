// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "common/CF.hpp"

#include "ui/graphics/AboutCFDialog.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

AboutCFDialog::AboutCFDialog(QWidget * parent)
  : QDialog(parent)
{
  this->setWindowTitle("About CF");
  QString os = QString("%1 [%2bits]").arg(CF3_OS_LONGNAME).arg(CF3_OS_BITS);

  m_main_layout = new QVBoxLayout(this);
  m_info_layout = new QFormLayout();

  m_bt_ok = new QPushButton("OK");

  m_lab_qwt = new QLabel("COOLFluiD client is based in part on the "
                        "work of the Qwt project "
                        "(<a href=\"http://qwt.sf.net\">http://qwt.sf.net</a>).", this);

  m_lab_qwt->setTextFormat(Qt::RichText);
  m_lab_qwt->setWordWrap(true);
  m_lab_qwt->setOpenExternalLinks(true);

  m_info_layout->addRow( "CF version:", new QLabel(CF3_VERSION_STR) );
  m_info_layout->addRow( "Kernel version:", new QLabel(CF3_KERNEL_VERSION_STR) );
  m_info_layout->addRow( "Build operating system:", new QLabel(os) );
  m_info_layout->addRow( "Build processor:", new QLabel(CF3_BUILD_PROCESSOR) );
  m_info_layout->addRow( "Qt version:", new QLabel(QT_VERSION_STR) );
  m_info_layout->addRow( "Build time:", new QLabel(__DATE__ " - " __TIME__) );

  m_main_layout->addLayout(m_info_layout);
  m_main_layout->addWidget(m_lab_qwt);
  m_main_layout->addWidget(m_bt_ok);

  connect(m_bt_ok, SIGNAL(clicked()), this, SLOT(accept()));
}

//////////////////////////////////////////////////////////////////////////

AboutCFDialog::~AboutCFDialog()
{
  delete m_bt_ok;
  delete m_lab_qwt;
  delete m_info_layout;
  delete m_main_layout;
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
