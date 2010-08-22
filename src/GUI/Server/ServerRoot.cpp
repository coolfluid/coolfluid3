#include <QtCore>

#include "GUI/Network/ComponentNames.hpp"

#include "Common/CRoot.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Server/ProcessingThread.hpp"
#include "GUI/Server/CSimulator.hpp"

#include "GUI/Server/ServerRoot.hpp"

using namespace std;
using namespace CF::Common;
using namespace CF::GUI::Server;

ProcessingThread * ServerRoot::m_thread = CFNULL;
QMutex ServerRoot::m_mutex;
SignalCatcher * ServerRoot::m_catcher = new SignalCatcher();
boost::shared_ptr<XmlDoc> ServerRoot::m_doc;

void SignalCatcher::finished()
{
  ServerRoot::getCore()->sendSignal(*ServerRoot::m_thread->getNode().document());
  delete ServerRoot::m_thread;
  ServerRoot::m_thread = CFNULL;
  ServerRoot::m_doc.reset();
  ServerRoot::m_mutex.unlock();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CRoot::Ptr & ServerRoot::getRoot()
{
  static bool rootCreated = false;
  static CRoot::Ptr root = CRoot::create(SERVER_ROOT);
  static CCore::Ptr core(new CCore());
  static CSimulator::Ptr simulator(new CSimulator());

  if(!rootCreated)
  {
    m_thread = CFNULL;
    root->add_component(core);
    root->add_component(simulator);

    rootCreated = true;

    simulator->createSimulator();
  }

  return root;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerRoot::processSignal(const string & target,
                               const CPath & receiver,
                               const string & clientid,
                               const string & frameid,
                               XmlNode & node, boost::shared_ptr<XmlDoc> doc)
{
  if(m_mutex.tryLock())
  {
    m_doc.swap(doc);
    Component::Ptr receivingCompo = getRoot()->access_component(receiver);
    m_thread = new ProcessingThread(node, target, receivingCompo);
    QObject::connect(m_thread, SIGNAL(finished()), m_catcher, SLOT(finished()));
    m_thread->start();
  }
  else
  {
    getCore()->sendFrameRejected(clientid, frameid, SERVER_CORE_PATH, "Server is busy.");
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CCore::Ptr ServerRoot::getCore()
{
  return boost::dynamic_pointer_cast<CCore>(getRoot()->access_component(SERVER_CORE_PATH));
}
