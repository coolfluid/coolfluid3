// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <exception>
#include <vector>
#include <string>

#include <QtXml>
#include <QStringList>

#include <boost/algorithm/string.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/CMethod.hpp"
#include "Common/CPath.hpp"
#include "Common/EventHandler.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/BasicExceptions.hpp"
//#include "Common/ConfigFileReader.hpp"

#include "Common/CoreEnv.hpp"
#include "Common/FactoryRegistry.hpp"
#include "Common/XmlHelpers.hpp"

#include "Mesh/CMesh.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/Neu/CReader.hpp"

//#include "Framework/GlobalStopCriteria.hpp"
//#include "Framework/Simulator.hpp"
//#include "Framework/SimulationStatus.hpp"

#include "GUI/Network/ComponentType.hpp"

#include "GUI/Server/RemoteClientAppender.hpp"
#include "GUI/Server/CSimulator.hpp"
#include "GUI/Server/ServerRoot.hpp"

using namespace MPI;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Neu;
//using namespace CF::Config;
//using namespace CF::Environment;
//using namespace CF::Framework;
using namespace CF::GUI::Server;
using namespace CF::GUI::Network;
using namespace std;

CSimulator::CSimulator(const QString & simulatorName)
  : Component(simulatorName.toStdString())
//: Maestro(simulatorName.toStdString()), QThread()
{
  BUILD_COMPONENT;

  TypeInfo::instance().regist<CSimulator>( type_name() );

  m_configured = false;
  m_lastSubsystemConfigured = -1;

  RemoteClientAppender * rca = new RemoteClientAppender();

  Logger::instance().getStream(Logger::ERROR).addStringForwarder(rca);
  Logger::instance().getStream(Logger::INFO).addStringForwarder(rca);

  connect(rca, SIGNAL(newData(const QString &)),
          this, SLOT(newData(const QString &)));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CSimulator::~CSimulator()
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CSimulator::run()
{
  if(m_lastSubsystemConfigured == -1)
    emit error("No subsystem configured");

  else
  {
    try
    {
      emit message("Starting the simulation");

      std::string subSysName;
      std::string subSysType;
      std::string subSys;
      std::vector<std::string> subsysnames;
      QStringList::iterator it = m_subsystemNames.begin();
      Common::SafePtr<EventHandler> event_handler = Common::CoreEnv::instance().getEventHandler();

      //      SimulationStatus& simStatus = SimulationStatus::instance();

      subSysName = m_subsystemNames.at(m_lastSubsystemConfigured).toStdString();
      subSysType = m_subsystemTypes.at(m_lastSubsystemConfigured).toStdString();
      subSys = subSysName + '\n' + subSysType + '\n';

      while(it != m_subsystemNames.end())
      {
        subsysnames.push_back(it->toStdString());
        it++;
      }

      //  simStatus.resetAll();
      //simStatus.setSubSystems(subsysnames);

//      if(m_stopcriteria.isNull())
//      {
//        emit error("Stop criteria is NULL...");
//        return;
//      }

     // COMM_WORLD.Barrier();
      CFinfo << "#\n###### RUN PHASE ####################\n#\n";
 //     for ( ; !m_stopcriteria->isSatisfied(); )
//      {
//        //simStatus.incrementNbIter();
//        event_handler->call_signal ( "CF_ON_MAESTRO_RUN", subSys );
//      }

      emit message("Simulation finished");
    }
    catch ( std::exception& e )
    {
      emit error(e.what());
    }
    catch (...)
    {
      emit error("Unknown exception thrown and not caught !!!\nAborting ...");
    }

  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CSimulator::loadCaseFile(const QString & filename)
{
  try
  {
    ConfigArgs args;
    //ConfigFileReader reader;

    //reader.parse(filename.toStdString(), args);

    //   this->createSimulator();

    //  m_simulator->openCaseFile(filename.toStdString());
    //this->configure(args);
    m_caseFile = filename;

    emit message("File loaded: " + filename);

    return true;
  }
  catch ( std::exception& e )
  {
    emit error(e.what());
  }
  catch (...)
  {
    emit error("Unknown exception thrown and not caught !!!\nAborting ...");
  }

  return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CSimulator::newData(const QString & data)
{
  emit message(data);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QStringList CSimulator::getConcreteTypes(const QString & abstractType)
const
{
  QStringList typesList;

  /// @bug what if the abstract type does not exist ????

  std::vector< ProviderBase* > registered_providers =
  CoreEnv::instance().getFactoryRegistry()->
  getFactory(abstractType.toStdString())->getAllProviders();

//  for(size_t i = 0; i < registered_providers.size(); ++i)
//    typesList << QString(registered_providers[i]->getProviderName().c_str());

  return typesList;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CSimulator::configureSimulator(const QDomDocument & document)
{
  throw NotImplemented(FromHere(), "ServerNetworkComm::configureSimulator");
//  QString filename;
//  QTemporaryFile tempFile;
//  QTextStream out;
//  std::string tree;
//  ConfigArgs args;
//  QDomDocument doc = document;

//  //  this->createSimulator();

//  if(doc.firstChild().nodeName() != "XCFcase")
//  {
//    QDomElement elem = doc.createElement("XCFcase");

//    elem.appendChild(doc.firstChild());
//    doc.appendChild(elem);
//  }

//  tree = doc.toString().toStdString();
//  args = ConverterTools::xmlToConfigArgs(tree);

//  tempFile.open();
//  filename = tempFile.fileName();

//  out.setDevice(&tempFile);

//  out << ConverterTools::xmlToCFcase(tree).c_str();

//  tempFile.close();

//  this->loadCaseFile(filename);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CSimulator::getTreeXML() const
{
//  XMLNode parent = XMLNode::createXMLTopNode("XCFcase");
//  m_rootComponent->xml_tree(parent);
//  return parent.createXMLString();
  return "";
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int CSimulator::getSubsystemsCount() const
{
  return m_subsystemNames.size();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CSimulator::runConfigPhase(int subsystem)
{
  bool configured = false;

  if(subsystem > m_subsystemNames.size())
    emit error("Unknown subsystem");
  else
  {
    try
    {
//      std::string subSysName = m_subsystemNames.at(subsystem).toStdString();
//      std::string subSysType = m_subsystemTypes.at(subsystem).toStdString();
//      std::string subSys = subSysName + '\n' + subSysType + '\n';
//      Common::SafePtr<EventHandler> event_handler = Common::CoreEnv::instance().getEventHandler();
//
//      // we manage the simulator
//      //this->manage(m_simulator);
//
//      CFinfo << "+++++++++++++++++++++++++++++++ CONFIGURING with " << __FUNCTION__
//      << " ++++++++++++++++++++++++++++++++\n" << CFendl;
//
//      CFinfo << "#\n###### STARTING SUBSYSTEM : " << subSysName << " ######\n#\n";
//      CFinfo << "in " << __FUNCTION__ << " at line " << __LINE__ << "\n" << CFendl;
//      event_handler->call_signal("CF_ON_MAESTRO_BUILDSUBSYSTEM", subSys);
//
//      //    COMM_WORLD.Barrier();
//
//      CFinfo << "#\n###### CONFIG PHASE #################\n#\n";
//      event_handler->call_signal("CF_ON_MAESTRO_CONFIGSUBSYSTEM", subSys);
//
//      //    COMM_WORLD.Barrier();
//
//      CFinfo << "#\n###### SOCKETS PLUG PHASE ###########\n#\n";
//      event_handler->call_signal("CF_ON_MAESTRO_PLUGSOCKETS", subSys);
//
//      //    COMM_WORLD.Barrier();
//
//      CFinfo << "#\n###### BUILD PHASE ##################\n#\n";
//      event_handler->call_signal("CF_ON_MAESTRO_BUILDMESHDATA", subSys);
//
//      //    COMM_WORLD.Barrier();
//
//      CFinfo << "#\n###### SETUP PHASE ##################\n#\n";
//      event_handler->call_signal("CF_ON_MAESTRO_SETUP", subSys);

     // COMM_WORLD.Barrier();

//      m_lastSubsystemConfigured = subsystem;

      configured = true;
    }
    catch ( std::exception& e )
    {
      emit error(e.what());
    }
    catch (...)
    {
      emit error("Unknown exception thrown and not caught !!!\nAborting ...");
    }
  }
  return configured;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CSimulator::runUnconfigPhase()
{
  bool unconfigured = false;

  if(m_lastSubsystemConfigured == -1)
    emit error("No subsystem configured");
  else
  {
    try
    {
//      std::string subSysName = m_subsystemNames.at(m_lastSubsystemConfigured).toStdString();
//      std::string subSysType = m_subsystemTypes.at(m_lastSubsystemConfigured).toStdString();
//      std::string subSys = subSysName + '\n' + subSysType + '\n';
//      Common::SafePtr<EventHandler> event_handler = Common::CoreEnv::instance().getEventHandler();
//
//      CFinfo << "#\n###### UNSETUP PHASE ################\n#\n";
//      event_handler->call_signal ( "CF_ON_MAESTRO_UNSETUP", subSys );
//
      //    COMM_WORLD.Barrier();
//
//      CFinfo << "#\n###### SOCKETS UNPLUG PHASE #########\n#\n";
//      event_handler->call_signal ( "CF_ON_MAESTRO_UNPLUGSOCKETS", subSys );
//
      //    COMM_WORLD.Barrier();
//
//      CFinfo << "#\n###### DESTRUCTION SUBSYSTEM PHASE #########\n#\n";
//      event_handler->call_signal ( "CF_ON_MAESTRO_DESTROYSUBSYSTEM", subSys );
//
      //COMM_WORLD.Barrier();
//
//      m_lastSubsystemConfigured = -1;

      unconfigured = true;
    }
    catch ( std::exception& e )
    {
      emit error(e.what());
    }
    catch (...)
    {
      emit error("Unknown exception thrown and not caught !!!\nAborting ...");
    }
  }

  return unconfigured;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int CSimulator::readSubSystems()
{
  // which subsystems will be controlled by meastro
  std::vector<std::string> subsysnames;// = m_simulator->getSubSystemNames();
  std::vector<std::string> subsystypes;// = m_simulator->getSubSystemTypes();

  cf_assert (subsysnames.size() == subsystypes.size());

  m_subsystemNames.clear();
  m_subsystemTypes.clear();

  for ( Uint i = 0; i < subsysnames.size(); ++i )
  {
    m_subsystemNames << subsysnames[i].c_str();
    m_subsystemTypes << subsystypes[i].c_str();
  }

  return m_subsystemNames.size();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CSimulator::getSubSystem(int subSystem) const
{
  return m_subsystemNames.at(subSystem) + " " + m_subsystemTypes.at(subSystem);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CF::Common::CRoot::Ptr CSimulator::root() const
{
  return m_root;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CSimulator::createSimulator()
{
  CRoot::Ptr root = ServerRoot::getRoot();

  CGroup::Ptr flowGroup = root->create_component_type<CGroup>("Flow");
  CGroup::Ptr mgGroup = root->create_component_type<CGroup>("MG");
  CGroup::Ptr solidGroup = root->create_component_type<CGroup>("Solid");

  // Flow subcomponents
  CLink::Ptr meshLnk = flowGroup->create_component_type<CLink>("Mesh");
  CMethod::Ptr fvm = flowGroup->create_component_type<CMethod>("FVM");
  CMethod::Ptr petsc = flowGroup->create_component_type<CMethod>("Petsc");

  // MG subcomponents
  CMesh::Ptr mesh1 = mgGroup->create_component_type<CMesh>("Mesh1");
  CMesh::Ptr mesh2 = mgGroup->create_component_type<CMesh>("Mesh2");

  // Solid subcomponents
  CMesh::Ptr mesh3 = solidGroup->create_component_type<CMesh>("Mesh3");
  CMesh::Ptr mesh4 = solidGroup->create_component_type<CMesh>("Mesh4");
  CLink::Ptr petscLnk = solidGroup->create_component_type<CLink>("PestcLnk");

  mesh4->create_component_type<CReader>("NeuMeshReader");

  meshLnk->link_to(mesh1);
  petscLnk->link_to(petsc);
}
