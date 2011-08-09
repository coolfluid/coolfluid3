// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options/errors.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Common/MPI/CPEManager.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/ListeningThread.hpp"

#include "Tools/Solver/CWorker.hpp"
#include "Tools/Solver/LogForwarder.hpp"

using namespace boost;
using namespace Comm;
using namespace CF;
using namespace CF::Common;
using namespace CF::Common::MPI;
using namespace CF::Tools::Solver;

int main(int argc, char ** argv)
{
  boost::program_options::options_description desc("Allowed options");
  std::string forward = "none";
  LogForwarder * forwarder;
  Communicator parent_comm;
  int rank;

  desc.add_options()
      ("forward", program_options::value<std::string>(&forward)->default_value(forward),
       "Defines whether log should be forwarded to the parent process. Three "
       "values available: none (no forwarding), rank0 (only rank 0 forwards) or "
       "all (all ranks forward).");

  // initiate the CF core and MPI environment
  Core::instance().initiate(argc, argv);
  PE::instance().init(argc, argv);


  parent_comm = PE::instance().get_parent();
  rank = PE::instance().rank();

  if( parent_comm == MPI_COMM_NULL )
  {
    CFerror << "This solver cannot run without a manager. Exiting..." << CFendl;

    return 1;
  }

  // get command line arguments
//  program_options::variables_map vm;
//  program_options::store(program_options::parse_command_line(argc, argv, desc), vm);
//  program_options::notify(vm);

  // create the PE manager
  CPEManager::Ptr mgr = Core::instance().root().get_child_ptr("Tools")->
      create_component_ptr<CPEManager>("PEManager");

  Core::instance().root().create_component_ptr<CWorker>("Worker");

  mgr->mark_basic();

  // set the forwarder, if needed
//  if( forward == "all" || (forward == "rank0" && PE::instance().rank() == 0) )
  {
    forwarder = new LogForwarder();
    Logger::instance().getStream(INFO).addStringForwarder(forwarder);
  }

  Logger::instance().getStream( INFO ).setFilterRankZero(false);
  CFinfo << "Worker[" << rank << "] -> " << getpid() << " Syncing with the parent..." << CFendl;
//  PE::instance().get_parent()->barrier();
  PE::instance().barrier();
  MPI_Barrier( parent_comm );
  CFinfo << "Worker[" << rank << "] -> " << getpid() << " Synced with the parent!" << CFendl;

  mgr->listening_thread().join();

//  PE::instance().barrier();

  CFinfo << "Worker[" << rank << "] -> " << "C U..." << CFendl;

  // synchronize with the parent before we finalize the MPI environment
  MPI_Barrier( parent_comm );

  // terminate the CF core and MPI environment
  PE::instance().finalize();
  Core::instance().terminate();

  return 0;
}
