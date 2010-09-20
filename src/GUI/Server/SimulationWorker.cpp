// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtCore>
#include <QHostInfo>
#include <string>
#include <mpi.h>

#include <QDomDocument>

//#include "logcpp/Appender.hpp"
//#include "logcpp/PatternLayout.hpp"

#include "Common/Log.hpp"
#include "Common/WorkerStatus.hpp"

#include "GUI/Server/RemoteClientAppender.hpp"
#include "GUI/Server/CSimulator.hpp"

#include "GUI/Server/SimulationWorker.hpp"

using namespace std;
using namespace MPI;
using namespace CF::Common;
//using namespace CF::Config;
using namespace CF::GUI::Server;

SimulationWorker::SimulationWorker()
{
  m_managerComm = COMM_WORLD.Get_parent();
  m_rank = COMM_WORLD.Get_rank();
  m_srvSimulation = CFNULL; // = new CSimulator("Simulator");

  //  this->srvSimulation = new CSimulator("Simulator");

  //  connect(this->srvSimulation, SIGNAL(message(const QString &)),
  //          this, SLOT(sendToParent(const QString &)));
  //
  //  connect(this->srvSimulation, SIGNAL(finished()),
  //          this, SLOT(simulationFinished()));

  this->createSimulator();

  m_listener.addComm(m_managerComm);

//  connect(&m_listener, SIGNAL(newFrame(const MPI::Intercomm &,
//                                             const CF::Common::BuilderParserFrameInfo &)),
//          this, SLOT(newFrame(const MPI::Intercomm &,
//                              const CF::Common::BuilderParserFrameInfo &)));

  this->setBarrier(m_managerComm);
  this->setStatus(WorkerStatus::IDLE);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::listen()
{
  m_listener.start();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString SimulationWorker::getLineHeader() const
{
  return QString("Worker[%1][%2] ").arg(m_subSystemName).arg(m_rank);
}

/****************************************************************************

PRIVATE METHODS

****************************************************************************/

//void SimulationWorker::buildAndSend(const BuilderParserFrameInfo & fi)
//{
//  string frame;

//  if(!BuilderParser::buildFrame(fi, m_protocol, frame))
//    qDebug() << BuilderParser::getErrorString().c_str() << fi.frameAttributes.size();
//  else
//    m_managerComm.Send(frame.c_str(), frame.length() + 1, CHAR, 0, 0);
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::sendToParent(const QString & message)
{
  throw NotImplemented(FromHere(), "SimulationWorker::sendToParent");
//  BuilderParserFrameInfo fi;
//  QString msgToSend = message.trimmed();

//  msgToSend.replace('\n', QString("\n%1").arg(this->getLineHeader()));
//  msgToSend.prepend(this->getLineHeader());

//  fi.setFrameType(MGR_WKR_STRING);
//  fi.frameAttributes["value"] = msgToSend.toStdString();

//  this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void SimulationWorker::ack(ManagerWorkerFrameType type)
//{
//  BuilderParserFrameInfo fi;

//  fi.setFrameType(MGR_WKR_ACK);
//  fi.frameAttributes["workerRank"] = QString::number(m_rank).toStdString();
//  fi.frameAttributes["type"] = m_protocol.getTypeName(type);

//  this->buildAndSend(fi);
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::runSimulation()
{
//  int subSysCount = m_srvSimulation->readSubSystems();
//  QString error;

//  if(subSysCount == 0)
//    error = "No subsystem found";

//  if(!m_srvSimulation->runConfigPhase(0))
//    error = "An error occured during the setup phase. Simulation could not be run.";

//  if(!error.isEmpty())
//  {
//    //   this->commServer->sendError(clientId, error);
//  }
//  else
//  {
//    if(m_rank == 0)
//    {
//      BuilderParserFrameInfo frameInfo;

//      frameInfo.setFrameType(MGR_WKR_TREE);
//      frameInfo.frameData = XMLNode::parseString(m_srvSimulation->getTreeXML().toStdString().c_str());
//      this->buildAndSend(frameInfo);
//    }

//    this->setBarrier(COMM_WORLD, WorkerStatus::RUNNING);
//    m_srvSimulation->run();
//    this->setBarrier(COMM_WORLD);
//    this->simulationFinished();

//    if(m_rank == 0)
//      this->ack(MGR_WKR_SIMULATE);

//    this->setStatus(WorkerStatus::IDLE);
//  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::simulationFinished()
{
  m_srvSimulation->runUnconfigPhase();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void SimulationWorker::newFrame(const Intercomm & senderComm,
//                                const BuilderParserFrameInfo & frameInfo)
//{
//  static bool configOk = false;
//  //  static bool runSim = false;

//  switch(frameInfo.frameType)
//  {
//    case MGR_WKR_SET_SUBSYS:
//      m_subSystemName = frameInfo.frameAttributes.find("name")->second.c_str();
//      m_subSystemType = frameInfo.frameAttributes.find("type")->second.c_str();
//      CFinfo << "Assigned to solve \"" << m_subSystemName.toStdString()
//      << "\" subsystem on " << QHostInfo::localHostName().toStdString() << " "
//      << "(PID = " << getpid() << ").\n" << CFflush;
//      this->ack(MGR_WKR_SET_SUBSYS);
//      this->setBarrier(m_managerComm, WorkerStatus::IDLE);
//      break;

//    case MGR_WKR_CONFIGURE:
//    {
//      //    if(configOk)
//      {
//        //     this->createSimulator();
//        //     this->srvSimulation->readSubSystems();
//      }

//      //    if(!configOk)
//      {
//        this->setStatus(WorkerStatus::CONFIGURING);
//        QDomDocument document;
//        CFLogInfo (  "Received simulation tree.\n");

//        document.setContent(QString(frameInfo.frameData.createXMLString()));

//        m_srvSimulation->configureSimulator(document);
//        this->ack(MGR_WKR_CONFIGURE);
//        configOk = true;
//        this->setBarrier(COMM_WORLD, WorkerStatus::IDLE);
//      }
//      break;
//    }

//    case MGR_WKR_SIMULATE:
//      //    if(!runSim)
//    {
//      this->runSimulation();
//      //     runSim = true;
//    }

//      break;

//    case MGR_WKR_OPEN_PORT:
//      this->openPort(frameInfo.getAttribute("remoteRole"));
//      break;

//    case MGR_WKR_CONNECT:
//      this->connectToPort(frameInfo.getAttribute("portName"),
//                          frameInfo.getAttribute("remoteRole"));
//      break;

//    case MGR_WKR_EXIT:
//      this->setStatus(WorkerStatus::EXITING);
//      CFinfo << "Exiting.\n";
//      m_listener.stopListening();
//      m_listener.wait(100);
//      m_srvSimulation->exit(0);
//      this->ack(MGR_WKR_EXIT);
//      this->setBarrier(m_managerComm, WorkerStatus::IDLE);
//      qApp->exit(0);
//      break;
//  }
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::openPort(const string & remoteRole)
{
  throw NotImplemented(FromHere(), "ServerNetworkComm::openPort");
//  char myPort[MPI_MAX_PORT_NAME];
//  Intercomm remoteComm;
//  BuilderParserFrameInfo frameInfo;

//  if(m_rank == 0)
//  {
//    Open_port(MPI_INFO_NULL, myPort); // open a port and build the port name

//    // send the port name to the manager
//    //  this->parent.Send(myPort, sizeof(myPort), CHAR, 0, 0);
//    frameInfo.setFrameType(MGR_WKR_PORT_NAME);
//    frameInfo.frameAttributes["value"] = myPort;
//  }

//  // broadcast the port name to all "buddy workers"
//  // if this->rank == 0, the string is sent; otherwise the string is received
//  COMM_WORLD.Bcast(myPort, sizeof(myPort), CHAR, 0);

//  m_managerComm.Barrier();

//  remoteComm = COMM_WORLD.Accept(myPort, MPI_INFO_NULL, 0);

//  if(m_rank == 0)
//  {
//    CFinfo << "Worker[" << m_subSystemName.toStdString() << "] " << "Found " <<
//    remoteComm.Get_remote_size() << " \"" << remoteRole << "\" process(es).\n";
//  }

//  remoteComm.Barrier();

//  m_comms[remoteRole.c_str()] = remoteComm;

//  m_managerComm.Barrier();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::connectToPort(const string & portName,
                                     const string & remoteRole)
{
  Intercomm remoteComm;

  remoteComm = COMM_WORLD.Connect(portName.c_str(), MPI_INFO_NULL, 0);

  if(m_rank == 0)
  {
    CFinfo << "Worker[" << m_subSystemName.toStdString() << "] " << "Found " <<
    remoteComm.Get_remote_size() << " \"" << remoteRole << "\" process(es).\n";
  }

  remoteComm.Barrier();

  m_comms[remoteRole.c_str()] = remoteComm;

  m_managerComm.Barrier();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::setStatus(WorkerStatus::Type status)
{
  throw NotImplemented(FromHere(), "ServerNetworkComm::setStatus");

  //  if(this->rank == 0)
//  {
//    BuilderParserFrameInfo fi;

//    PEInterface::instance().change_status(status);
//    fi.setFrameType(MGR_WKR_STATUS);

//    fi.frameAttributes["subSysName"] = m_subSystemName.toStdString();
//    fi.frameAttributes["workerRank"] = QString::number(m_rank).toStdString();
//    fi.frameAttributes["value"] = WorkerStatus::Convert::to_str(status);

//    this->buildAndSend(fi);
//  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::setBarrier(Intercomm comm)
{
  WorkerStatus::Type oldStatus = PEInterface::instance().status();

  this->setStatus(WorkerStatus::WAITING);
  comm.Barrier();
  this->setStatus(oldStatus);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::setBarrier(Intercomm comm, WorkerStatus::Type newStatus)
{
  this->setStatus(WorkerStatus::WAITING);
  comm.Barrier();
  this->setStatus(newStatus);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SimulationWorker::createSimulator()
{
  delete m_srvSimulation;
  m_srvSimulation = new CSimulator("Simulator");

  connect(m_srvSimulation, SIGNAL(message(const QString &)),
          this, SLOT(sendToParent(const QString &)));

  connect(m_srvSimulation, SIGNAL(finished()),
          this, SLOT(simulationFinished()));
}
