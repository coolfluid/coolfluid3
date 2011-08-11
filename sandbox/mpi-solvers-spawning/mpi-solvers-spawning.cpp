// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CF.hpp"
#include "Common/Core.hpp"
#include "Common/CGroup.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/CPEManager.hpp"

using namespace CF::Common;
using namespace CF::Common::mpi;

int main(int argc, char * argv[])
{
  Core::instance().initiate( argc, argv );
  PE::instance().init(argc, argv);

  CPEManager & manager = Core::instance().tools().create_component<CPEManager>("PEManager");

  manager.spawn_group("Group1", 1, "../../src/Tools/Solver/coolfluid-solver");

  CFinfo << "============================================================================" << CFendl;

  manager.spawn_group("Group2", 1, "../../src/Tools/Solver/coolfluid-solver");

  PE::instance().finalize();
  Core::instance().terminate();
  return 0;
}
