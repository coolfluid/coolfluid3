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

PE::PE(int argc, char** args)
{
  init();
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
  //if (m_comm!=nullptr) throw THROW ALREADY INITIALIZED
  MPI_Init(&argc,&args);
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
    MPI_Finalize();
    m_comm=nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////

void PE::barrier()
{
  if ( is_init() ) MPI_Barrier(m_comm);
}

////////////////////////////////////////////////////////////////////////////////

Uint PE::rank() const
{
  if ( !is_init() ) return 0;
  int irank;
  MPI_Comm_rank(m_comm,&irank);
  return static_cast<Uint>(irank);
}

////////////////////////////////////////////////////////////////////////////////

Uint PE::size() const
{
    if ( !is_init() ) return 1;
    int nproc;
    MPI_Comm_size(m_comm,&nproc);
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

  }  // common
} // CF
