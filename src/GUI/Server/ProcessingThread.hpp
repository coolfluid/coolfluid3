#ifndef CF_GUI_Server_ProcessingThread_hpp
#define CF_GUI_Server_ProcessingThread_hpp

////////////////////////////////////////////////////////////////////////////

#include <QThread>

#include "Common/XmlHelpers.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {

  ////////////////////////////////////////////////////////////////////////////

  class ProcessingThread : public QThread
  {

  public:

    ProcessingThread(CF::Common::XmlNode & m_node, const std::string & target,
                     boost::shared_ptr<CF::Common::Component> receiver);

    void run();

    CF::Common::XmlNode & getNode() const;

  private:

      CF::Common::XmlNode & m_node;

      std::string m_target;

      boost::shared_ptr<CF::Common::Component> m_receiver;
};

  ////////////////////////////////////////////////////////////////////////////

} // namespace Server
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_ProcessingThread_hpp
