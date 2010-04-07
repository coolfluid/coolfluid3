#ifndef PEINTERFACEMPI_HH
#define PEINTERFACEMPI_HH

#include <list>

#include <mpi.h>

#include "Common/CommonAPI.hh"

#include "Common/PEInterface.hh"
#include "Common/NonCopyable.hh"
#include "Common/MPI/MPIDataTypeHandler.hh"

namespace CF  {
    namespace Common {

       class MPIInitObject;
       class MPIDataTypeHandler;


/// Parallel Environment interface for MPI
/// Cannot be in the MPI namespace because it is a specialization of
/// a class in the Common namespace
class Common_API PEInterface
  : public PEInterfaceBase,
    public Common::NonCopyable < PEInterface >
{
  private:
    MPI_Comm Comm;
   // MPIDataTypeHandler DataTypeHandler;

    typedef std::list<MPIInitObject *> InitContainerType;
    std::list<MPIInitObject *> InitList_;
    bool InitOK_;
    bool StopCalled_;

    public:
   /// constructor
   ///    Needs argc, args because of MPI
  PEInterface (int * argc, char *** args, MPI_Comm UsedCom = MPI_COMM_WORLD);

   /// Destructor
   ~PEInterface ();

    /// Set the barrier
    void setBarrier()  {   MPI_Barrier(GetCommunicator());  }

   /// Returns the total number of execution contexts in
   /// the universum (not the big one of course, but the
   /// cluster-one ;-) )
  inline unsigned int GetProcessorCount () const;

   /// Return the ID of this processor (between 0 and GetProcessorCount)
  inline unsigned int GetRank () const;


   /// Return true if this is a parallel simulation in some way
  inline bool IsParallel () const;

   /// return true if the model is capable of multiple cpu's
  inline bool IsParallelCapable () const;

   /// return the name of the model
  inline std::string GetName () const;

   /// init- en finalize functions
   /// The object will be freed by the PE (should
   /// become a ModulePointer)
  void RegisterInitObject (MPIInitObject * Obj);

   /// advance communication
   /// This function should be called from time to time when there are
   /// overlapping
   /// communication requests (beginSync() is called but not endSync() )
   /// in orde to give the hardware/implementation a change to advance the
   /// communication.
  inline void AdvanceCommunication ();

   /// Return the communicator to be used
   /// (for now we always work in MPI_COMM_WORLD, but this could change
  inline MPI_Comm GetCommunicator () const;
protected:

  void CallInitFunctions ();
  void CallDoneFunctions ();
  void CallInitFunction (MPIInitObject * M) const;
  void CallDoneFunction (MPIInitObject * M) const;

private:
  MPI_Errhandler ErrHandler_;

};

//////////////////////////////////////////////////////////////////////////////

inline void PEInterface::AdvanceCommunication ()
{
    int Flag;
    MPI_Iprobe (MPI_ANY_SOURCE, MPI_ANY_TAG, GetCommunicator(), &Flag,
    MPI_STATUS_IGNORE);
}

inline MPI_Comm PEInterface::GetCommunicator () const
{
    return MPI_COMM_WORLD;
}

inline bool PEInterface::IsParallelCapable () const
{
    return true;
}

inline std::string PEInterface::GetName () const
{
    return "MPI";
}

unsigned int PEInterface::GetProcessorCount () const
{
    int Size;
    MPI_Comm_size (Comm, &Size);
    return static_cast<unsigned>(Size);
}

unsigned int PEInterface::GetRank () const
{
    static int Rank = -1;
    if (Rank != -1)
return Rank;

    MPI_Comm_rank (Comm, &Rank);
    return static_cast<unsigned int>(Rank);
}

bool PEInterface::IsParallel () const
{
    return (GetProcessorCount() > 1);
}

//////////////////////////////////////////////////////////////////////////////

  } // Common

} // COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif
