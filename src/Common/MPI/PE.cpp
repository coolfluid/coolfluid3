// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"

#include "Common/BasicExceptions.hpp"
#include "Common/MPI/PE.hpp"

//#include "Common/MPI/debug.hpp"

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////////

PE::PE(int argc, char** args)
{
  m_comm = nullptr;
  init(argc,args);
  m_current_status=WorkerStatus::NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

PE::PE()
{
  m_comm = nullptr;
  m_current_status = WorkerStatus::NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

PE::~PE()
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

bool PE::is_init() const
{
  int is_initialized = 0;
  MPI_CHECK_RESULT(MPI_Initialized,(&is_initialized));
  return bool(is_initialized);
}

////////////////////////////////////////////////////////////////////////////////

bool PE::is_finalized() const
{
  int is_finalized = 0;
  MPI_CHECK_RESULT(MPI_Finalized,(&is_finalized));
  return bool(is_finalized);
}

////////////////////////////////////////////////////////////////////////////////

std::string PE::version() const
{
  int version = 0;
  int subversion = 0;
  MPI_CHECK_RESULT(MPI_Get_version,(&version,&subversion));
  return std::string( to_str(version) + "." + to_str(subversion) );
}

////////////////////////////////////////////////////////////////////////////////

void PE::initialized(int argc, char** args)
{
  if ( is_finalized() )
    throw SetupError( FromHere(), "Should not call PE::initialize() after PE::finalize()" );

  if( !is_initialized() && !is_finalized() ) // then initialize
  {
    MPI_CHECK_RESULT(MPI_Init,(&argc,&args));
    //  CFinfo << "MPI (version " <<  version() << ") -- initiated" << CFendl;
  }

  m_comm = MPI_COMM_WORLD;
}

////////////////////////////////////////////////////////////////////////////////

void PE::finalize()
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

void PE::barrier()
{
  if ( is_active() ) MPI_CHECK_RESULT(MPI_Barrier,(m_comm));
}

////////////////////////////////////////////////////////////////////////////////

Uint PE::rank() const
{
  if ( !is_active() ) return 0;
  int irank;
  MPI_CHECK_RESULT(MPI_Comm_rank,(m_comm,&irank));
  return static_cast<Uint>(irank);
}

////////////////////////////////////////////////////////////////////////////////

Uint PE::size() const
{
  if ( !is_active() ) return 1;
  int nproc;
  MPI_CHECK_RESULT(MPI_Comm_size,(m_comm,&nproc));
  return static_cast<Uint>(nproc);
}


////////////////////////////////////////////////////////////////////////////////

void PE::change_status(WorkerStatus::Type status) 
{
  cf_assert ( WorkerStatus::Convert::instance().is_valid(status) );
  m_current_status = status;
}

////////////////////////////////////////////////////////////////////////////////

WorkerStatus::Type PE::status() 
{
  return m_current_status;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace mpi
} // namespace Common
} // namespace CF



