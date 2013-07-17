// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"

#include "common/BasicExceptions.hpp"
#include "common/PE/Comm.hpp"

//#include "common/PE/debug.hpp"

namespace cf3 {
namespace common {
namespace PE {

////////////////////////////////////////////////////////////////////////////////

Comm::Comm(int argc, char** args)
{
  m_comm = nullptr;
  init(argc,args);
  m_current_status=WorkerStatus::NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

Comm::Comm()
{
  m_comm = nullptr;
  m_current_status = WorkerStatus::NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

Comm::~Comm()
{
  finalize();
}

////////////////////////////////////////////////////////////////////////////////

Comm& Comm::instance()
{
  static Comm comm_instance;
  return comm_instance;
}

////////////////////////////////////////////////////////////////////////////////

bool Comm::is_initialized() const
{
  int is_initialized = 0;
  MPI_CHECK_RESULT(MPI_Initialized,(&is_initialized));
  return bool(is_initialized);
}

////////////////////////////////////////////////////////////////////////////////

bool Comm::is_finalized() const
{
  int is_finalized = 0;
  MPI_CHECK_RESULT(MPI_Finalized,(&is_finalized));
  return bool(is_finalized);
}

////////////////////////////////////////////////////////////////////////////////

std::string Comm::version() const
{
  int version = 0;
  int subversion = 0;
  MPI_CHECK_RESULT(MPI_Get_version,(&version,&subversion));
  return std::string( to_str(version) + "." + to_str(subversion) );
}

////////////////////////////////////////////////////////////////////////////////

void Comm::init(int argc, char** args)
{
  if ( is_finalized() )
    throw SetupError( FromHere(), "Should not call Comm::initialize() after Comm::finalize()" );

  if( !is_initialized() ) // then initialize
  {
    MPI_CHECK_RESULT(MPI_Init,(&argc,&args));
    //  CFinfo << "MPI (version " <<  version() << ") -- initiated" << CFendl;
  }

  m_comm = MPI_COMM_WORLD;
}

////////////////////////////////////////////////////////////////////////////////

void Comm::finalize()
{
  if( is_initialized() && !is_finalized() ) // then finalized
  {
    MPI_CHECK_RESULT(MPI_Finalize,());
    //  CFinfo << "MPI (version " <<  version() << ") -- finalized" << CFendl;
  }

  m_comm = nullptr;

//    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
//    int is_mpi_finalized;
//    MPI_CHECK_RESULT(MPI_Finalized,(&is_mpi_finalized));
//    while (is_mpi_finalized!=true){
//      MPI_CHECK_RESULT(MPI_Finalized,(&is_mpi_finalized));
//      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
//    }

}

////////////////////////////////////////////////////////////////////////////////

void Comm::barrier()
{
  if ( is_active() ) MPI_CHECK_RESULT(MPI_Barrier,(m_comm));
}

////////////////////////////////////////////////////////////////////////////////

void Comm::barrier( Communicator comm )
{
  cf3_assert( comm != MPI_COMM_NULL );

  if ( is_active() ) MPI_CHECK_RESULT(MPI_Barrier,(comm));

}

////////////////////////////////////////////////////////////////////////////////

Uint Comm::rank() const
{
  if ( !is_active() ) return 0;
  int irank;
  MPI_CHECK_RESULT(MPI_Comm_rank,(m_comm,&irank));
  return static_cast<Uint>(irank);
}

////////////////////////////////////////////////////////////////////////////////

Uint Comm::size() const
{
  if ( !is_active() ) return 1;
  int nproc;
  MPI_CHECK_RESULT(MPI_Comm_size,(m_comm,&nproc));
  return static_cast<Uint>(nproc);
}


////////////////////////////////////////////////////////////////////////////////

void Comm::change_status(WorkerStatus::Type status)
{
  cf3_assert ( WorkerStatus::Convert::instance().is_valid(status) );
  m_current_status = status;
}

////////////////////////////////////////////////////////////////////////////////

WorkerStatus::Type Comm::status()
{
  return m_current_status;
}

////////////////////////////////////////////////////////////////////////////////

Communicator Comm::spawn( int count, const char * command, char ** args,
                            const char * hosts )
{
  ::MPI::Info info = ::MPI::Info::Create();
  Communicator comm;

  if(count < 1)
    throw BadValue(FromHere(), "Cannot spawn less than 1 process.");

  char * cmd_non_const = new char[strlen(command) + 1];
  std::strcpy(cmd_non_const, command);
  int error_codes[count];

  info.Set("host", "localhost");

  CFinfo << "Spawning " << count << " workers on localhost." << CFendl;

  MPI_Comm_spawn(cmd_non_const,   // command to run
                 args,            // arguments to the command
                 count,           // number of processes
                 info,            // infos
                 0,               // manager (root) rank
                 m_comm,
                 &comm,
                 error_codes);

  return comm;
}

////////////////////////////////////////////////////////////////////////////////

Communicator Comm::get_parent() const
{
  PE::Communicator comm;
  MPI_CHECK_RESULT(MPI_Comm_get_parent,(&comm));
  return comm;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace PE
} // namespace common
} // namespace cf3



