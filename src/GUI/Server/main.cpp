// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <map>

#include <QtCore>
#include <QXmlDefaultHandler>
#include <QHostInfo>

#include "Common/MPI/PEInterface.hpp"

#include "GUI/Network/NetworkException.hpp"
#include "GUI/Network/HostInfos.hpp"
#include "GUI/Server/ServerRoot.hpp"
#include "GUI/Server/SimulationWorker.hpp"

#include "Common/CoreEnv.hpp"
//#include "Framework/Simulator.hpp"
#include "Common/DirPaths.hpp"
#include "Common/CoreVars.hpp"

#define CF_NO_TRACE

using namespace MPI;
using namespace CF::GUI::Network;
using namespace CF::GUI::Server;

using namespace CF;
using namespace CF::Common;
//using namespace CF::Config;
//using namespace CF::Environment;
//using namespace CF::Framework;

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  QString errorString;
  int return_value = 0;
  int port = 62784;
  char hostfile[] = "machine.txt";

  QList<HostInfos> list;

  AssertionManager::instance().AssertionDumps = true;
  AssertionManager::instance().AssertionThrows = true;

  //  setenv("OMPI_MCA_orte_default_hostfile", hostfile, 1);

  /// @todo the following line should be in PE::Init_PE()
  // setenv("OMPI_MCA_orte_default_hostfile", "./machine.txt", 1);

  /// @todo init MPI environment here

  QFile file(hostfile);

  file.open(QFile::ReadOnly);

  QTextStream in(&file);

  while(!in.atEnd())
  {
    HostInfos hi;
    QString host = in.readLine();
    QStringList line = host.split(" ");

    hi.m_hostname = line.at(0);

    for(int i = 1 ; i < line.size() ; i++)
    {
      QStringList param = line.at(i).split("=");

      if(param.at(0) == "slots")
        hi.m_nbSlots = param.at(1).toUInt();

      else if(param.at(0) == "max-slots")
        hi.m_maxSlots = param.at(1).toUInt();
    }

    list.append(hi);
  }

  try
  {
    CoreEnv& cf_env = CoreEnv::instance();  // build the environment
    cf_env.initiate ( argc, argv );        // initiate the environemnt
    ConfigArgs args;
    //cf_env.configure(args);
    cf_env.setup();

    std::vector<std::string> moduleDirs;

    moduleDirs.push_back("../../../dso/");

    DirPaths::instance().addModuleDirs(moduleDirs);

   /* if(COMM_WORLD.Get_parent() != COMM_NULL)
    {
      SimulationWorker worker;
      worker.listen();
      return app.exec();
    }*/

    if(argc == 2)
    {
      std::istringstream iss(argv[1]);

      if((iss >> std::dec >> port).fail())
        errorString = "Port number is not a valid integer\n";
      else if(port < 49153 || port > 65535)
        errorString = "Port number must be an integer between 49153 and 65535\n";
    }

    if(errorString.isEmpty())
    {
      QHostInfo hostInfo = QHostInfo::fromName(QHostInfo::localHostName());
      CCore::Ptr sk = ServerRoot::getCore();
      QString message("Server successfully launched on machine %1 (%2) on port %3!");

      //        sk->listenToNetwork(hostInfo.addresses().at(0).toString(), port);
      sk->listenToNetwork(hostInfo.addresses().last().toString(), port);
      sk->setHostList(list);

      message = message.arg(hostInfo.addresses().at(0).toString())
                .arg(QHostInfo::localHostName())
                .arg(port);

      std::cout << message.toStdString() << std::endl;

      return_value = app.exec();
    }


    // unsetup the runtime environment
    cf_env.unsetup();
    // terminate the runtime environment
    cf_env.terminate();

  }
  catch(NetworkException ne)
  {
    errorString = QString("%1\nAborting ... \n").arg(ne.what());
    return_value = -1;
  }
  catch(std::string str)
  {
    errorString = QString("%1\nAborting ... \n").arg(str.c_str());
    return_value = -1;
  }
  catch ( std::exception& e )
  {
    errorString = QString("%1\nAborting ... \n").arg(e.what());
    return_value = 1;
  }
  catch (...)
  {
    errorString = "Unknown exception thrown and not caught !!!\n";
    errorString += "Aborting ... \n";
    return_value = 1;
  }

  if(!errorString.isEmpty())
  {
    std::cerr << std::endl << std::endl;
    std::cerr << "Server application exited on error:" << std::endl;
    std::cerr << errorString.toStdString() << std::endl << std::endl;
  }

  return return_value;
}
