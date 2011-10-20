// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options/errors.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/Log.hpp"

#include "common/PE/CPEManager.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/ListeningThread.hpp"

#include "Tools/Solver/CWorker.hpp"
#include "Tools/Solver/LogForwarder.hpp"
#include "Tools/Solver/Notifier.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::Tools::Solver;

int main(int argc, char ** argv)
{
  LogForwarder * forwarder;
  Notifier * notifier;
  Communicator parent_comm;
  int rank;

  // initiate the CF core and MPI environment
  Core::instance().initiate(argc, argv);
  Comm::instance().init(argc, argv);

  parent_comm = Comm::instance().get_parent();
  rank = Comm::instance().rank();

  if( parent_comm == MPI_COMM_NULL )
  {
    CFerror << "This solver cannot run without a manager. Exiting..." << CFendl;
    return 1;
  }

  // create the PE manager
  CPEManager::Ptr mgr = Core::instance().root().get_child_ptr("Tools")->
      create_component_ptr<CPEManager>("PEManager");

  Core::instance().root().create_component_ptr<CWorker>("Worker");
  
  // Make sure the python ScriptEngine gets created, if it exists
  try
  {
    build_component("CF.Python.ScriptEngine", "DummyScriptEngine");
  }
  catch(...)
  {
  }

  mgr->mark_basic();

  notifier = new Notifier( mgr );

  notifier->listen_to_event("tree_updated", true);

  // set the forwarder, if needed
  //  if( forward == "all" || (forward == "rank0" && Comm::instance().rank() == 0) )
    {
      forwarder = new LogForwarder();
      Logger::instance().getStream(INFO).addStringForwarder(forwarder);
    }

  bool rank0 = CFinfo.getFilterRankZero(LogStream::SCREEN);

  CFinfo.setFilterRankZero(LogStream::SCREEN, false);

  CFinfo << "Worker[" << rank << "] -> Syncing with the parent..." << CFendl;
  Comm::instance().barrier();
  MPI_Barrier( parent_comm );
  CFinfo << "Worker[" << rank << "] -> Synced with the parent!" << CFendl;

  CFinfo.setFilterRankZero(LogStream::SCREEN, rank0);

  mgr->listening_thread().join();

  CFinfo << "Worker[" << rank << "] -> " << "C U..." << CFendl;

  // synchronize with the parent before we finalize the MPI environment
  MPI_Barrier( parent_comm );

  // terminate the CF core and MPI environment
  Comm::instance().finalize();
  Core::instance().terminate();

  delete forwarder;
  delete notifier;

  return 0;
}
