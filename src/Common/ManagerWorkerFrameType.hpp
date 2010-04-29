#ifndef CF_Common_ManagerWorkerFrameType_hpp
#define CF_Common_ManagerWorkerFrameType_hpp

#include "Common/CommonAPI.hpp"

namespace CF {

namespace Common {

  enum ManagerWorkerFrameType
  {
  /// Type id used to indicate that a frame has no type (i.e. it
  /// does not respect the protocol).
  MGR_WKR_NO_TYPE,

  /// Type id for the frame root tag.
  MGR_WKR_FRAME_ROOT,

  MGR_WKR_CONNECT,

  MGR_WKR_OPEN_PORT,

  MGR_WKR_PORT_NAME,

  MGR_WKR_CONFIGURE,

  MGR_WKR_SIMULATE,

  MGR_WKR_ACK,

  MGR_WKR_STRING,

  MGR_WKR_SET_SUBSYS,

  MGR_WKR_EXIT,

  MGR_WKR_STATUS,

  MGR_WKR_TREE,

  MGR_WKR_QUIT

  }; // enum ManagerWorkerFrameType

} // namespace Common
} // namespace CF

#endif // CF_Common_ManagerWorkerFrameType_hpp
