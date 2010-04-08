#ifndef PARALLEL_MPI_COMMPATTERN_HH
#define PARALLEL_MPI_COMMPATTERN_HH

#include <mpi.h>

#include "Common/CF.hh"

namespace CF {
namespace Common  {
namespace MPI  {

//////////////////////////////////////////////////////////////////////////////

/// base class for communication patterns
///   stores communicator and constants
class CommPattern
{
public:

    /// value type: first=receive from, second=send to
    typedef std::pair<int,int> value_type;


    /// Constructor
    CommPattern ( MPI_Comm comm = MPI_COMM_SELF );

    /// Return the communicator to be used in communications
    /// (could be different from the one the pattern was initialised with
    MPI_Comm GetCommunicator () const;

    /// Return the communicator size
    int GetCommSize () const
    {
      return m_size;
    }

    /// Return our rank
    int GetRank () const
    {
      return m_rank;
    }

protected:

    MPI_Comm m_communicator;

    int m_rank;
    int m_size;
};

template <typename ITERATOR>
class CommPatternBase : public CommPattern
{
public:

    typedef ITERATOR const_iterator;

    /// constructor
    CommPatternBase ( MPI_Comm comm ) : CommPattern (comm)
    {
    }

    /// Return an iterator for the given rank (default=our rank)
    const_iterator begin (int rank = -1) const
    {
      return const_iterator (*this, rank, false);
    }

    const_iterator end (int rank = -1) const
    {
      return const_iterator (*this, rank, true);
    }
};

//////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

#endif
