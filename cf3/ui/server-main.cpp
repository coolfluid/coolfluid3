// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/program_options/errors.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <coolfluid-paths.hpp>

#include "common/Group.hpp"
#include "common/Environment.hpp"
#include "common/NetworkInfo.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/Manager.hpp"

#include "ui/server/ServerExceptions.hpp"
#include "ui/server/ServerRoot.hpp"

#include "common/Core.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::common::PE;
using namespace cf3::ui::server;

int main(int argc, char *argv[])
{
  std::string error_string;
  int return_value = 0;
  int port = 62784;
  Uint nb_workers = 1;
  std::string hostfile("./machine.txt");

  boost::program_options::options_description desc("Allowed options");

  desc.add_options()
      ("help", "Prints this help message and exits")
      ("port", program_options::value<int>(&port)->default_value(port),
           "Port to use for network communications.")
      ("np", program_options::value<Uint>(&nb_workers)->default_value(nb_workers),
           "Number of MPI workers to spawn.")
      ("hostfile", program_options::value<std::string>(&hostfile)->default_value(hostfile),
           "MPI hostfile.");


  AssertionManager::instance().AssertionDumps = true;
  AssertionManager::instance().AssertionThrows = true;

  // tell the CF core the the server is running
  Core::instance().network_info().start_server();

  try
  {
    Core& cf_env = Core::instance();  // build the environment

    // get command line arguments
    program_options::variables_map vm;
    program_options::store(program_options::parse_command_line(argc, argv, desc), vm);
    program_options::notify(vm);

    if ( vm.count("help") > 0 )
    {
      std::cout << "Usage: " << argv[0] << " [--port <port-number>] "
                   "[--np <workers-count>] [--hostfile <hostfile>]" << std::endl;
      std::cout << desc << std::endl;
      return 0;
    }

    // setup COOLFluiD environment
    // cf_env.set_mpi_hostfile("./machine.txt"); // must be called before MPI_Init !
    cf_env.initiate ( argc, argv );        // initiate the environemnt

    PE::Comm::instance().init( argc, argv );
    ServerRoot::instance().root();

    if( Comm::instance().size() != 1 )
      error_string = "This application is not designed to run in parallel.";

    if( nb_workers == 0 )
      error_string = "At least 1 worker must be spawn.";

    // spawn the MPI workers
    Handle< Manager > mgr(Core::instance().tools().get_child("PEManager"));
    /// @warning the below instruction will be broken when installing CF
    mgr->spawn_group("Workers", nb_workers, CF3_BUILD_DIR "/cf3/Tools/solver/coolfluid-solver");

    std::cout << getpid() << std::endl;

    // check if the port number is valid and launch the network connection if so
    if(port < 49153 || port > 65535)
      error_string = "Port number must be an integer between 49153 and 65535\n";
    else if( error_string.empty() )
    {
//      Core::instance().network_info().set_hostname( QHostInfo::localHostName().toStdString() );
//      Core::instance().network_info().set_port( port );

      Handle< CCore > sk = ServerRoot::instance().core();

      sk->listen_to_port(port); // start listening to the network
    }

    PE::Comm::instance().finalize();
    // terminate the runtime environment
    cf_env.terminate();

  }
  catch(program_options::error & error)
  {
    error_string = error.what();
  }
  catch(NetworkError ne)
  {
    error_string = ne.what();
  }
  catch(std::string str)
  {
    error_string = str.c_str();
  }
  catch ( std::exception& e )
  {
    error_string = e.what();
  }
  catch (...)
  {
    error_string = "Unknown exception thrown and not caught !!!\n";
  }

  if(!error_string.empty())
  {
    std::cerr << std::endl << std::endl;
    std::cerr << "Server application exited on error:" << std::endl;
    std::cerr << error_string << std::endl;
    std::cerr << "Aborting ..." << std::endl << std::endl << std::endl;
    return_value = -1;
  }

  // tell the CF core that the server is about to exit
  Core::instance().network_info().stop_server();

  return return_value;
}
