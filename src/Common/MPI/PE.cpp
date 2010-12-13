// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/thread/thread.hpp>

#include "Common/MPI/PE.hpp"
#include "Common/Log.hpp"

using namespace boost;

namespace CF {
namespace Common  {

////////////////////////////////////////////////////////////////////////////////

PE::PE(int argc, char** args):
	boost::mpi::communicator()
{
  m_environment=new mpi::environment(argc,args);
  m_current_status=WorkerStatus::NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

PE::PE(): 
	boost::mpi::communicator()
{
  m_environment = nullptr;
  m_current_status = WorkerStatus::NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

PE::~PE () 
{
  finalize();
}

////////////////////////////////////////////////////////////////////////////////

PE& PE::instance() 
{
  static PE pe_instance;
  return pe_instance;
}

////////////////////////////////////////////////////////////////////////////////

void PE::init(int argc, char** args) 
{
// The mpi::environment object is initialized with the program arguments
// (which it may modify) in your main program. The creation of this object
// initializes MPI, and its destruction will finalize MPI.
	
  if (!m_environment)
    delete_ptr(m_environment);
  m_environment = new mpi::environment(argc,args);
}

////////////////////////////////////////////////////////////////////////////////

bool PE::is_init() const
{
  if ( m_environment == nullptr ) return false;
  return m_environment->initialized();
}

////////////////////////////////////////////////////////////////////////////////

void PE::finalize()
{
	if ( is_init() )
	{
    //barrier();
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    delete_ptr(m_environment);
    m_environment=nullptr;
		
		/// @TODO Communicator has no destructor, see boost/mpi/communicator.hpp, 
		///       this is sort of dangerous.
		///       Needs to be checked if it somehow realizes that
	}	
}

////////////////////////////////////////////////////////////////////////////////

void PE::barrier()
{
	if ( is_init() )
	{
		mpi::communicator::barrier();
	}	
}

////////////////////////////////////////////////////////////////////////////////

Uint PE::rank() const 
{
  if ( !is_init() ) return 0;
  return static_cast<Uint>(mpi::communicator::rank());
}

////////////////////////////////////////////////////////////////////////////////

Uint PE::size() const
{
  if ( !is_init() ) return 1;
  return static_cast<Uint>(mpi::communicator::size());
}


////////////////////////////////////////////////////////////////////////////////

void PE::change_status(WorkerStatus::Type status) 
{
  cf_assert ( WorkerStatus::Convert::is_valid(status) );
  m_current_status = status;
}

////////////////////////////////////////////////////////////////////////////////

WorkerStatus::Type PE::status() 
{
  return m_current_status;
}

////////////////////////////////////////////////////////////////////////////////

  }  // common
} // CF
