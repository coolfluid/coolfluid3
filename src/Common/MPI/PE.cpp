// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/MPI/PE.hpp"
#include "Common/MPI/tools.hpp"
#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////////

PE::PE(int argc, char** args)
{
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

void PE::init(int argc, char** args) 
{
  if (m_comm!=nullptr) throw SetupError(FromHere(),"Trying to re-initialize PE (parallel environment) without calling finalize.");
  MPI_CHECK_RESULT(MPI_Init,(&argc,&args));
  m_comm=MPI_COMM_WORLD;
}

////////////////////////////////////////////////////////////////////////////////

bool PE::is_init() const
{
  if ( m_comm == nullptr ) return false;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void PE::finalize()
{
  if ( is_init() )
  {
//    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    MPI_CHECK_RESULT(MPI_Finalize,());
    m_comm=nullptr;
//    int is_mpi_finalized;
//    MPI_CHECK_RESULT(MPI_Finalized,(&is_mpi_finalized));
//    while (is_mpi_finalized!=true){
//      MPI_CHECK_RESULT(MPI_Finalized,(&is_mpi_finalized));
//      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
//    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void PE::barrier()
{
  if ( is_init() ) MPI_CHECK_RESULT(MPI_Barrier,(m_comm));
}

////////////////////////////////////////////////////////////////////////////////

Uint PE::rank() const
{
  if ( !is_init() ) return 0;
  int irank;
  MPI_CHECK_RESULT(MPI_Comm_rank,(m_comm,&irank));
  return static_cast<Uint>(irank);
}

////////////////////////////////////////////////////////////////////////////////

Uint PE::size() const
{
  if ( !is_init() ) return 1;
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
