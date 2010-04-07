#include <mpi.h>

#include "FakePE.hh"

using namespace CF::Common;

FakePE::FakePE() : m_is_init (false)
{
}

FakePE::~FakePE()
{
 if (m_is_init) finalize();
}

FakePE& FakePE::get_instance()
{
  static FakePE instance;
  return instance;
}

void FakePE::init()
{
 MPI::Init (); // init MPI environment
 
 m_is_init = true;
}

void FakePE::finalize()
{
 MPI::Finalize();
 m_is_init = false;
}

bool FakePE::is_init()
{
  return m_is_init;
}

int FakePE::get_rank()
{
 if(m_is_init)
  return MPI::COMM_WORLD.Get_rank();
 else
  return 0;
}

int FakePE::get_size()
{
 if(m_is_init)
  return MPI::COMM_WORLD.Get_size();
 else
  return 1;
}

void FakePE::set_barrier()
{
 if(m_is_init)
   MPI::COMM_WORLD.Barrier();
}