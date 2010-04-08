#include "CommPattern.hh"

namespace CF {
namespace Common  {
namespace MPI  {

//////////////////////////////////////////////////////////////////////////////

CommPattern::CommPattern (MPI_Comm C)
    : m_communicator (C)
{
    MPI_Comm_size (m_communicator, &m_size);
    MPI_Comm_rank (m_communicator, &m_rank);
}

MPI_Comm CommPattern::GetCommunicator () const
{
    return m_communicator;
}


//////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF
