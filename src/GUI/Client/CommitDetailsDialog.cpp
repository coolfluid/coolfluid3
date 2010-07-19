#include <QtGui>

#include "GUI/Client/CommitDetails.hpp"

#include "GUI/Client/CommitDetailsDialog.hpp"

using namespace CF::GUI::Client;

CommitDetailsDialog::CommitDetailsDialog(QWidget * parent)
: QDialog(parent)
{
  //CommitDetails details;
  m_mainLayout = new QVBoxLayout(this);
  m_buttonBox = new QDialogButtonBox(this);
  m_view = new QTableView(this);

  m_buttonBox->addButton(QDialogButtonBox::Ok);

  m_view->resizeRowsToContents();
  //this->resize(this->width() * 2, this->height());

  m_mainLayout->addWidget(m_view, 0);
  m_mainLayout->addWidget(m_buttonBox, 1);

  this->setFixedSize(this->width(), this->height());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CommitDetailsDialog::~CommitDetailsDialog()
{
 delete m_mainLayout;
 delete m_buttonBox;
 delete m_view;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CommitDetailsDialog::show(CommitDetails & details)
{
  if (details.rowCount() > 0)
  {
    m_view->setModel(&details);
    this->exec();
  }
}
