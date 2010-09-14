#include <QAction>
#include <QFileIconProvider>
#include <QMenu>

#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Client/NMeshReader.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

NMeshReader::NMeshReader(const QString & name)
  : CNode(name, "CMeshReader", MESH_READER_NODE)
{
  BUILD_COMPONENT;

  CFinfo << ClientRoot::browser()->get_child_count();

  QAction * action;

  action = new QAction("Read", m_contextMenu);
  connect(action, SIGNAL(triggered()), this, SLOT(readMesh()));
  m_contextMenu->addAction(action);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NMeshReader::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Trashcan);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NMeshReader::getToolTip() const
{
  return this->getComponentType();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NMeshReader::readMesh()
{
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlOps::add_signal_frame(*XmlOps::goto_doc_node(*doc.get()),
                           "read", full_path(),
                           full_path(), false);

  ClientRoot::core()->sendSignal(*doc);
}

