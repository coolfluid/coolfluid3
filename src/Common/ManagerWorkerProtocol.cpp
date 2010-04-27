#include "Common/ManagerWorkerFrameType.hpp"

#include "Common/ManagerWorkerProtocol.hpp"

using namespace CF::Common;

ManagerWorkerProtocol::ManagerWorkerProtocol() :
BuilderParserRules(MGR_WKR_NO_TYPE, MGR_WKR_FRAME_ROOT, "MgrWkrXml")
{
  this->buildRules();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ManagerWorkerProtocol::buildRules()
{
  this->setTypeName(MGR_WKR_CONNECT, "connect");
  this->addAttribute(MGR_WKR_CONNECT, "portName", MAND_OPTIONAL);
  this->addAttribute(MGR_WKR_CONNECT, "remoteRole", MAND_MANDATORY);
  
  this->setTypeName(MGR_WKR_OPEN_PORT, "openPort");
  this->addAttribute(MGR_WKR_OPEN_PORT, "remoteRole", MAND_MANDATORY);
  
  this->setTypeName(MGR_WKR_PORT_NAME, "portName");
  this->addAttribute(MGR_WKR_PORT_NAME, "value", MAND_MANDATORY);
  
  this->setTypeName(MGR_WKR_CONFIGURE, "configure");
  this->setDataMandatoriness(MGR_WKR_CONFIGURE, MAND_MANDATORY);
  
  this->setTypeName(MGR_WKR_SIMULATE, "simulate");
  
  this->setTypeName(MGR_WKR_ACK, "ack");
  this->addAttribute(MGR_WKR_ACK, "workerRank", MAND_MANDATORY);
  this->addAttribute(MGR_WKR_ACK, "type", MAND_MANDATORY);
  
  this->setTypeName(MGR_WKR_STRING, "string");
  this->addAttribute(MGR_WKR_STRING, "value", MAND_MANDATORY);
  
  this->setTypeName(MGR_WKR_SET_SUBSYS, "setSubSys");
  this->addAttribute(MGR_WKR_SET_SUBSYS, "name", MAND_MANDATORY);
  this->addAttribute(MGR_WKR_SET_SUBSYS, "type", MAND_MANDATORY);
  
  this->setTypeName(MGR_WKR_STATUS, "status");
  this->addAttribute(MGR_WKR_STATUS, "subSysName", MAND_OPTIONAL);
  this->addAttribute(MGR_WKR_STATUS, "workerRank", MAND_MANDATORY);
  this->addAttribute(MGR_WKR_STATUS, "value", MAND_MANDATORY);
  
  this->setTypeName(MGR_WKR_TREE, "tree");
  this->setDataMandatoriness(MGR_WKR_TREE, MAND_MANDATORY);
  
  this->setTypeName(MGR_WKR_EXIT, "exit");
  
  this->setTypeName(MGR_WKR_QUIT, "quit");
}
