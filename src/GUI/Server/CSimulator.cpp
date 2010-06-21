#include <exception>
#include <vector>
#include <string>

#include <mpi.h>

#include <QtXml>
#include <QStringList>

#include <boost/algorithm/string.hpp>

//#include "logcpp/PatternLayout.hpp"

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/CPath.hpp"
#include "Common/EventHandler.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/BasicExceptions.hpp"
//#include "Common/ConfigFileReader.hpp"

#include "Common/CoreEnv.hpp"
#include "Common/FactoryRegistry.hpp"

//#include "Framework/GlobalStopCriteria.hpp"
//#include "Framework/Simulator.hpp"
//#include "Framework/SimulationStatus.hpp"

#include "GUI/Network/ComponentType.hpp"

#include "GUI/Server/RemoteClientAppender.hpp"
#include "GUI/Server/CSimulator.hpp"

using namespace MPI;
using namespace CF::Common;
//using namespace CF::Config;
//using namespace CF::Environment;
//using namespace CF::Framework;
using namespace CF::GUI::Server;
using namespace CF::GUI::Network;
using namespace std;

CSimulator::CSimulator(const QString & simulatorName)
//: Maestro(simulatorName.toStdString()), QThread()
{
 // logcpp::PatternLayout * f_layout;
//  logcpp::Appender * remote_appender;

  this->createSimulator();

  // m_simulator = /*NULL;//*/new Simulator(simulatorName.toStdString());

  //  this->createSimulator();

  //f_layout = new logcpp::PatternLayout();
  //remote_appender = new RemoteClientAppender("RemoteClientAppender" );

  //f_layout->setConversionPattern( "%m" );//%p
  //remote_appender->setLayout(f_layout);

  //CFLogger::instance().getMainLogger().addAppender(remote_appender);

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

      COMM_WORLD.Barrier();
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

void CSimulator::addComponent(const QString & path,
                                    ComponentType::Type type,
                                    const QString & name)
{
  boost::shared_ptr<Component> compo;

  try
  {
    compo = m_rootComponent->look_component(path.toStdString())->get();

    switch(type)
    {
      case ComponentType::GROUP :
      {
        boost::shared_ptr<Component> newCompo(new CGroup(name.toStdString()));
        compo->add_component(newCompo);
        break;
      }

      case ComponentType::LINK :
      {
        boost::shared_ptr<CLink> newCompo(new CLink(name.toStdString()));
        compo->add_component(newCompo);
        break;
      }

      default:
        throw ShouldNotBeHere(FromHere(), "Can not use this type");
        break;
    }

    emit message(QString("Added a new component named \"%1\" of type \"%2\" to '%3'")
                 .arg(name).arg(ComponentType::Convert::to_str(type).c_str()).arg(path));
    emit treeUpdated();

  }
  catch (CF::Common::Exception e)
  {
    emit error(e.what());
  }
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

      COMM_WORLD.Barrier();

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
      COMM_WORLD.Barrier();
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

  //  this->subsystemNames << "Fluid";
  //  this->subsystemTypes << "Fluid";
  //  this->subsystemNames << "Mesh";
  //  this->subsystemTypes << "Mesh";
  //  this->subsystemNames << "Solid";
  //  this->subsystemTypes << "Solid";

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

void CSimulator::createSimulator()
{
  //  CPath p;
  // @todo delete the old tree
  m_rootComponent = CRoot::create("Simulator");


  //m_rootComponent->complete_path(p);

  //qDebug() << m_rootComponent->full_path().string().c_str();
  //  m_rootComponent->access_component(CPath("/Root"));
  //  if(this->simulator.isNotNull())
  //   this->simulator.release();
  //
  //m_simulator = new Simulator("Simulator");
}
