// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CF.hpp"
#include "common/Core.hpp"
#include "common/Group.hpp"
#include "common/Log.hpp"
#include "common/Root.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/CPEManager.hpp"

using namespace cf3::common;
using namespace cf3::common::mpi;

int main(int argc, char * argv[])
{
  Core::instance().initiate( argc, argv );
  Comm::instance().init(argc, argv);

  CPEManager & manager = Core::instance().tools().create_component<CPEManager>("PEManager");

  manager.spawn_group("Group1", 1, "../../src/Tools/Solver/coolfluid-solver");

  CFinfo << "============================================================================" << CFendl;

  manager.spawn_group("Group2", 1, "../../src/Tools/Solver/coolfluid-solver");

  Comm::instance().finalize();
  Core::instance().terminate();
  return 0;
}
