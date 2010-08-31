#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "Common/CPath.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/SelectPathDialog.hpp"

#include "GUI/Client/SelectPathPanel.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

SelectPathPanel::SelectPathPanel(const QString & path, QWidget *parent) :
    QWidget(parent)
{
  m_btBrowse = new QPushButton("Browse", this);
  m_editPath = new QLineEdit(path, this);

  m_layout = new QHBoxLayout(this);

  m_layout->addWidget(m_editPath);
  m_layout->addWidget(m_btBrowse);

  connect(m_btBrowse, SIGNAL(clicked()), this, SLOT(btBrowseClicked()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SelectPathPanel::~SelectPathPanel()
{
  delete m_btBrowse;
  delete m_editPath;
  delete m_layout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SelectPathPanel::getValueString() const
{
  return m_editPath->text();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SelectPathPanel::setValue(const QString & path)
{
  m_editPath->setText(path);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SelectPathPanel::btBrowseClicked()
{
  SelectPathDialog spd;

  CPath path = spd.show(m_editPath->text().toStdString());

  if(!path.empty())
    m_editPath->setText(path.string().c_str());
}
