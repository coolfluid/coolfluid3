// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options/errors.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "common/Core.hpp"
#include "common/Journal.hpp"
#include "common/Log.hpp"
#include "common/Table.hpp"

#include "common/PE/Manager.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/ListeningThread.hpp"

#include "solver/Plotter.hpp"

#include "Tools/solver/CWorker.hpp"
#include "Tools/solver/LogForwarder.hpp"
#include "Tools/solver/Notifier.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::solver;
using namespace cf3::Tools::solver;

///////////////////////////////////////////////////////////////////////////////

void setup_tree()
{
  Handle< Component > tools = Core::instance().root().get_child("Tools");
  Plotter & plotter = *tools->create_component< Plotter >("Plotter");
  Table<Real> & table = *tools->create_component< Table<Real> >("MyTable");
  tools->create_component< Journal >("Journal");

  table.set_row_size(8); // reserve 8 columns
  Table<Real>::Buffer buffer = table.create_buffer(8000);

  table.mark_basic();
  plotter.mark_basic();
  plotter.set_data_set( table.uri() );

  // fill the table
  std::vector<Real> row(8);

  for(Real value = 0.0 ; value != 1000.0 ; value += 1.0 )
  {
    row[0] = value / 1000;          // x
    row[1] = 0;                     // y
    row[2] = (value / 1000) - 1;    // z
    row[3] = std::sin(4 * row[0]);  // u
    row[4] = 0;                     // v
    row[5] = std::cos(4 * row[2]);  // w
    row[6] = 1000;                  // p
    row[7] = 278 * row[0];          // t

    buffer.add_row( row );
  }

  buffer.flush();
}

//////////////////////////////////////////////////////////////////////////////

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

  setup_tree();

  // create the PE manager
  Handle< Manager > mgr = Core::instance().root().get_child("Tools")->
      create_component<Manager>("PEManager");

  Core::instance().root().create_component<CWorker>("Worker");

  // Make sure the python ScriptEngine gets created, if it exists
  try
  {
    build_component("cf3.python.ScriptEngine", "DummyScriptEngine");
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

  mgr->listening_thread()->join();

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
