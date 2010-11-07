// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/MPI/PEInterface.hpp"
#include "Common/Log.hpp"

using namespace boost;

namespace CF {
  namespace Common  {

////////////////////////////////////////////////////////////////////////////////

PEInterface::PEInterface(int argc, char** args):
	boost::mpi::communicator()
{
  m_environment=new mpi::environment(argc,args);
	m_current_status=WorkerStatus::NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

PEInterface::PEInterface() :
	boost::mpi::communicator()
{
  m_environment=0;
  m_current_status=WorkerStatus::NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

PEInterface::~PEInterface () 
{
  finalize();
}

////////////////////////////////////////////////////////////////////////////////

PEInterface& PEInterface::instance() 
{
  static PEInterface pe_instance;
  return pe_instance;
}

////////////////////////////////////////////////////////////////////////////////

void PEInterface::init(int argc, char** args) 
{
// The mpi::environment object is initialized with the program arguments
// (which it may modify) in your main program. The creation of this object
// initializes MPI, and its destruction will finalize MPI.
	
  if (m_environment!=0) delete(m_environment);
  m_environment=new mpi::environment(argc,args);
}

////////////////////////////////////////////////////////////////////////////////

bool PEInterface::is_init() const {
  if (m_environment==0) return false;
  return m_environment->initialized();
}

////////////////////////////////////////////////////////////////////////////////

void PEInterface::finalize() {
	if (is_init())
	{
		barrier();
		delete(m_environment);
		m_environment=0;
		/// TODO: communicator has no destructor, see boost/mpi/communicator.hpp, this is sort of dangerous
		/// needs to be checked if it somehow realizes that
	}	
}

////////////////////////////////////////////////////////////////////////////////

Uint PEInterface::rank() const {
  if (!is_init()) return 0;
  return static_cast<Uint>(mpi::communicator::rank());
}

////////////////////////////////////////////////////////////////////////////////

Uint PEInterface::size() const
{
  if (!is_init()) return 1;
  return static_cast<Uint>(mpi::communicator::size());
}


////////////////////////////////////////////////////////////////////////////////

void PEInterface::change_status(WorkerStatus::Type status) {
  cf_assert ( WorkerStatus::Convert::is_valid(status) );
  m_current_status = status;
}

////////////////////////////////////////////////////////////////////////////////

WorkerStatus::Type PEInterface::status() {
  return m_current_status;
}

////////////////////////////////////////////////////////////////////////////////

  }  // namespace common
} // namespace CF
