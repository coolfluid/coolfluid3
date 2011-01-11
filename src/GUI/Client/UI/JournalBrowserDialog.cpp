// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>

#include <QMessageBox>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Client/Core/NJournal.hpp"
#include "GUI/Client/Core/NJournalBrowser.hpp"
#include "GUI/Client/Core/SignalNode.hpp"

#include "GUI/Client/UI/SignalInspectorDialog.hpp"

#include "GUI/Client/UI/JournalBrowserDialog.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

namespace CF {
namespace GUI {
namespace ClientUI {

JournalBrowserBuilder & JournalBrowserBuilder::instance()
{
  static JournalBrowserBuilder jbb;
  return jbb;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserBuilder::journalRequest(bool local)
{
  m_dialog->show(nullptr);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

JournalBrowserBuilder::JournalBrowserBuilder()
{
  m_dialog = new JournalBrowserDialog();

  connect(&JournalNotifier::instance(), SIGNAL(journalRequest(bool)),
          this, SLOT(journalRequest(bool)));
}

////////////////////////////////////////////////////////////////////////////

JournalBrowserDialog::JournalBrowserDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new NJournalBrowser(nullptr, this))
{
  m_view = new QTableView(this);
  m_buttons = new QDialogButtonBox(this);

  m_mainLayout = new QVBoxLayout(this);

  ClientRoot::instance().browser()->add_component(m_model);

  m_view->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_view->horizontalHeader()->setStretchLastSection(true);
  m_view->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_view->setModel(m_model.get());
  m_view->setAlternatingRowColors(true);

  m_buttons->addButton(QDialogButtonBox::Ok);
  m_btExecute = m_buttons->addButton("Execute", QDialogButtonBox::ActionRole);

  m_mainLayout->addWidget(m_view);
  m_mainLayout->addWidget(m_buttons);

  m_view->updateGeometry();
  updateGeometry();
  adjustSize();
  resize(childrenRect().size());

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(close()));
  connect(m_buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(btClicked(QAbstractButton*)));
  connect(m_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

JournalBrowserDialog::~JournalBrowserDialog()
{
  ClientRoot::instance().browser()->remove_component(m_model->name());
  delete m_view;
  delete m_buttons;
  delete m_mainLayout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserDialog::show(const Common::XmlNode *rootNode)
{
  m_model->setRootNode(rootNode);
  m_model->requestJournal();

  this->exec();
//  this->setVisible(true);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserDialog::doubleClicked(const QModelIndex & index)
{
  NJournalBrowser * model = (NJournalBrowser*)m_view->model();

  if(index.isValid() && model != nullptr)
  {
    SignalInspectorDialog sid;

    sid.show(model->signal(index));
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserDialog::btClicked(QAbstractButton *button)
{
  if(button == (QAbstractButton*)m_btExecute)
  {
    QModelIndex index = m_view->currentIndex();

    try
    {
      if(index.isValid())
      {
        std::stringstream ss;
        boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
        XmlNode & frameNode = *XmlOps::add_node_to(*XmlOps::goto_doc_node(*doc.get()), "tmp");
        NJournalBrowser * model = (NJournalBrowser*)m_view->model();
        XmlOps::deep_copy(*model->signal(index).node(), frameNode);

        XmlAttr * clientIdAttr = frameNode.first_attribute( XmlParams::tag_attr_clientid() );
        XmlAttr * frameIdAttr = frameNode.first_attribute( XmlParams::tag_attr_frameid() );

        // if found,  we remove the old client UUID.
        // (sending the signal automatically adds the correct UUID)
        if(clientIdAttr != nullptr)
          frameNode.remove_attribute(clientIdAttr);

        // if found,  we remove the old frame UUID...
        if(frameIdAttr != nullptr)
          frameNode.remove_attribute(frameIdAttr);

        // ...and we add a newly generated one
        ss << boost::uuids::random_generator()();
        XmlOps::add_attribute_to(frameNode, XmlParams::tag_attr_frameid(), ss.str() );

        ClientRoot::instance().core()->sendSignal(*doc.get());
      }
    }
    catch(Exception & e)
    {
      ClientRoot::instance().log()->addMessage(e.what());
    }
  }
}

////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
