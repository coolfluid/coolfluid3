#include "Common/Component.hpp"

#include "GUI/Server/ProcessingThread.hpp"

using namespace CF::Common;
using namespace CF::GUI::Server;

ProcessingThread::ProcessingThread(XmlNode & node, const std::string & target,
                                   Component::Ptr receiver)
  : m_node(node),
    m_target(target),
    m_receiver(receiver)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ProcessingThread::run()
{
  m_receiver->call_signal(m_target, *m_node.first_node() );
}

XmlNode & ProcessingThread::getNode() const
{
  return m_node;
}
