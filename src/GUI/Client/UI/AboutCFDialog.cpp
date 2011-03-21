// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "Common/CF.hpp"

#include "GUI/Client/UI/AboutCFDialog.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

//////////////////////////////////////////////////////////////////////////

AboutCFDialog::AboutCFDialog(QWidget * parent)
  : QDialog(parent)
{
  this->setWindowTitle("About CF");
  QString os = QString("%1 [%2bits]").arg(CF_OS_LONGNAME).arg(CF_OS_BITS);

  m_mainLayout = new QVBoxLayout(this);
  m_infoLayout = new QFormLayout();

  m_btOK = new QPushButton("OK");

  m_labQwt = new QLabel("COOLFluiD client is based in part on the "
                        "work of the Qwt project "
                        "(<a href=\"http://qwt.sf.net\">http://qwt.sf.net</a>).", this);

  m_labQwt->setTextFormat(Qt::RichText);
  m_labQwt->setWordWrap(true);
  m_labQwt->setOpenExternalLinks(true);

  m_infoLayout->addRow( "CF version:", new QLabel(CF_VERSION_STR) );
  m_infoLayout->addRow( "Kernel version:", new QLabel(CF_KERNEL_VERSION_STR) );
  m_infoLayout->addRow( "Build operating system:", new QLabel(os) );
  m_infoLayout->addRow( "Build processor:", new QLabel(CF_BUILD_PROCESSOR) );
  m_infoLayout->addRow( "Qt version:", new QLabel(QT_VERSION_STR) );
  m_infoLayout->addRow( "Build time:", new QLabel(__DATE__ " - " __TIME__) );

  m_mainLayout->addLayout(m_infoLayout);
  m_mainLayout->addWidget(m_labQwt);
  m_mainLayout->addWidget(m_btOK);

  connect(m_btOK, SIGNAL(clicked()), this, SLOT(accept()));
}

//////////////////////////////////////////////////////////////////////////

AboutCFDialog::~AboutCFDialog()
{
  delete m_btOK;
  delete m_labQwt;
  delete m_infoLayout;
  delete m_mainLayout;
}

//////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
