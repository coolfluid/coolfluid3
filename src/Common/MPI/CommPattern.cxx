#include "CommPattern.hh"

namespace CF {
    namespace Common {

//////////////////////////////////////////////////////////////////////////////

CommPattern::CommPattern (MPI_Comm C)
    : Communicator_ (C)
{
    MPI_Comm_size (Communicator_, &Size_);
    MPI_Comm_rank (Communicator_, &Rank_);
}

MPI_Comm CommPattern::GetCommunicator () const
{
    return Communicator_;
}


//////////////////////////////////////////////////////////////////////////////

    }
}
