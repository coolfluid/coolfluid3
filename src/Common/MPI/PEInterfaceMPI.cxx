#include "Common/MPI/PEInterfaceMPI.hh"
#include "Common/MPI/MPIInitObject.hh"
#include "Common/MPI/MPIHelper.hh"

#include "Common/Log.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
    namespace Common {

//////////////////////////////////////////////////////////////////////////////

static void ThrowMPI (MPI_Comm * Comm, int * Error, ...)
{
	DoMPIError (*Error);
}

//////////////////////////////////////////////////////////////////////////////

PEInterface::PEInterface (int * argc, char *** args, MPI_Comm UsedCom)
    : Comm(UsedCom), /*DataTypeHandler (UsedCom),*/ InitOK_(false), StopCalled_(false)
{
    int Ret = MPI_Init (argc, args);
    if (Ret != MPI_SUCCESS) throw std::string("MPI_Init failed!");
    InitOK_ = true;

    Common::CheckMPIStatus(MPI_Errhandler_create (ThrowMPI, &ErrHandler_));
    Common::CheckMPIStatus(MPI_Errhandler_set (Comm, ErrHandler_));

  //DataTypeHandler.InitTypes ();

    CallInitFunctions ();
}

//////////////////////////////////////////////////////////////////////////////

PEInterface::~PEInterface ()
{
    CallDoneFunctions ();

  //DataTypeHandler.DoneTypes ();

    MPI_Finalize ();
    InitOK_=false;
}

void PEInterface::CallInitFunctions ()
{
    CFLogDebug( "PEInterface::CallInitFunctions()\n");
    cf_assert (!StopCalled_);
    InitContainerType::iterator Cur = InitList_.begin();
    while (Cur != InitList_.end())
    {
	CallInitFunction (*Cur);
	++Cur;
    }
}

void PEInterface::CallDoneFunctions ()
{
    CFLogDebug( "PEInterface::CallDoneFunctions ()\n");
    InitContainerType::iterator Cur = InitList_.begin();
    while (Cur != InitList_.end())
    {
	CallDoneFunction (*Cur);
	++Cur;
    }
    StopCalled_ = true;
    // Clear the list
    InitList_.clear ();
}

void PEInterface::CallInitFunction (MPIInitObject * NewObj) const
{
    CFLogDebug( "Calling MPI_Init on " << NewObj << "\n");
    NewObj->MPI_Init (GetCommunicator());
}
void PEInterface::CallDoneFunction (MPIInitObject * NewObj) const
{
    CFLogDebug( "Calling MPI_Done on " << NewObj << "\n");
    NewObj->MPI_Done ();
}

void PEInterface::RegisterInitObject (MPIInitObject * NewObj)
{
    cf_assert (!StopCalled_);

    InitList_.push_back (NewObj);

    if (InitOK_)
    {
	/// Initialization is already started, so we can call init immediately
	CallInitFunction (InitList_.back());
    }
}

//////////////////////////////////////////////////////////////////////////////

    }
}
