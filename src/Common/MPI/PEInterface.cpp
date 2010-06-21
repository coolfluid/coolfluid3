#include "Common/MPI/PEInterface.hpp"

using namespace boost;

namespace CF {
  namespace Common  {

////////////////////////////////////////////////////////////////////////////////

PEInterface::PEInterface(int argc, char** args) {
  PEInterface();
  m_environment=new mpi::environment(argc,args);
  mpi::communicator();
}

////////////////////////////////////////////////////////////////////////////////

PEInterface::PEInterface() {
  m_environment=0;
  m_current_status=WorkerStatus::NOT_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////

PEInterface::~PEInterface () {
  finalize();
}

////////////////////////////////////////////////////////////////////////////////

PEInterface& PEInterface::instance() {
  static PEInterface pe_instance;
  return pe_instance;
}

////////////////////////////////////////////////////////////////////////////////

void PEInterface::init(int argc, char** args) {
  if (m_environment!=0) delete(m_environment);
  m_environment=new mpi::environment(argc,args);
  mpi::communicator();
}

////////////////////////////////////////////////////////////////////////////////

bool PEInterface::is_init(){
  if (m_environment==0) return false;
  return m_environment->initialized();
}

////////////////////////////////////////////////////////////////////////////////

void PEInterface::finalize() {
  delete(m_environment);
  m_environment=0;
  /// TODO: communicator has no destructor, see boost/mpi/communicator.hpp, this is sort of dangerous
  /// needs to be checked if it somehow realizes that
}

////////////////////////////////////////////////////////////////////////////////

Uint PEInterface::rank() {
  if (!is_init()) return 0;
  return (Uint)mpi::communicator::rank();
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
