/// IDEA:
///      - Lazy deletion of ghost points (wait until sync)
///      - Maybe change alter map -> hashmap?
///      - For shared memory implementation
///        need to check writes to ghost points? (overload operator)
/// TO REMEMBER:
///      - Adding/Removing  a point should be an easy operationche
///        (no collective com)
///      - Synchronisation cost should be abs. minimum
///  - Per-element overhead should be low
/// TODO: - Sync speed??
///       - Make helper class for sync; Allow for easy replace of
///         sync method
///       - Add collection of statistics (with configure option?)
///         Number of bytes sent/received, number of syncs, ...
///       - Data and Metadata are seperated for a reason
///         (in fact there are multiple reasons)
///       - Remove _NOT_FOUND -> change to exception
/// Changes:
///       - 30/04/2004: * Switch to new MPI Datatype framework
///       - 02/05/2004: * Add size parameter
///                       (needed because of failure to have a compile-time
///                        size of the state vectors)
///       - 06/10/2005: * Added support for continuous global index
///                     * Cleanup: removed old unsafe constructs
///                       and added more safety (foolproof) checks
///                     * Removed "using ..." (not allowed in headers)
/// @todo make assert inline function that checks and throws if neccesairy


#ifndef PAR_VECTOR_IMP_HH
#define PAR_VECTOR_IMP_HH

#include <mpi.h>
#include <set>
#include <sstream>
#include <fstream>
#include <iostream>
#include <limits>

#include "Common/CF.hh"
#include "Common/BasicExceptions.hh"
#include "Common/MPI/DataTypeHandler.hh"

#include "Common/ArrayAllocator.hh"

#include "Common/Log.hh"

#include "Common/MPI/HelperFuncs.hh"
#include "Common/PE.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {
namespace MPI  {

//////////////////////////////////////////////////////////////////////////////

#ifdef 0
void WriteCommPatternHelper (MPI_Comm Comm,
  CF::Uint LocalSize, CF::Uint GhostSize,
  std::vector<CF::Uint> & Sends,
  std::vector<CF::Uint> & Receives)
{
    // In order not to overwrite files from another parvector
    static CF::Uint Count = 0;

    CFAUTOTRACE;

    CFLogInfo ("Writing communication pattern...\n");

    int CommRank, CommSize;

    Common::CheckMPIStatus(MPI_Comm_size (Comm, &CommSize));
    Common::CheckMPIStatus(MPI_Comm_rank (Comm, &CommRank));

    std::ostringstream S;
    S << "parvector_pattern." << Count << ".dot";

    std::string Name = S.str();
    S.str(std::string ());

    MPI_File FileHandle;

    Common::CheckMPIStatus(MPI_File_open (Comm, const_cast<char *>(Name.c_str()),
    MPI_MODE_SEQUENTIAL|MPI_MODE_CREATE|MPI_MODE_WRONLY,
    MPI_INFO_NULL, &FileHandle));

    MPI_Barrier (Comm);

    Common::CheckMPIStatus(MPI_File_set_size (FileHandle, 0));

    MPI_Barrier (Comm);



    if (!CommRank)
    {
  S << "digraph parvector {\n";
    }

    S << "  P" << CommRank << " [label=\"CPU" << CommRank << " ("
  << LocalSize << "+" << GhostSize << ")\"];\n";
    for (CF::Uint i=0; i<static_cast<CF::Uint>(CommSize); ++i)
    {
  if (!Sends[i])
      continue;
  S << "  P" << CommRank << " -> P" << i << " [label=\""
      << Sends[i] << "\"];\n";
    }

    std::string Buf = S.str();
    Common::CheckMPIStatus(MPI_File_write_ordered (FileHandle,
    const_cast<char *>(Buf.c_str()),
    Buf.size(),
      MPI_CHAR, MPI_STATUS_IGNORE));
    S.str(std::string());


    if (!CommRank)
    {
  S << "}\n";
    }

    Buf = S.str();
    Common::CheckMPIStatus(MPI_File_write_ordered (FileHandle,
    const_cast<char *> (Buf.c_str()),
    Buf.size(),
      MPI_CHAR, MPI_STATUS_IGNORE));
    S.str(std::string());


    Common::CheckMPIStatus(MPI_File_close (&FileHandle));

    ++Count;
}
#endif


/// Template class
///***************************************************************************/
template < typename T >
class ParVectorMPI
{
public:
  //===============================================================
  //======================== PUBLIC types =========================
  //===============================================================

  /// The index type (inherited from ArrayAllocator)
  typedef typename COOLFluiD::Common::ArrayAllocator<T>::IndexType  IndexType;

  /// Represents the type stored in the vector
  typedef T _ElementType;

private:

  //=========================================================
  //============== Private structs & typedefs ===============
  //=========================================================

  /// Stores the internal data for an element
  /// (currently the global index, and - if unused - the
  /// next free element
  typedef struct
  {
    // GlobalIndex is also used for the free list
    IndexType GlobalIndex;
  } DataType;

  typedef std::map<IndexType,IndexType> TGhostMap;
  typedef std::map<IndexType,IndexType> TIndexMap;


  //============================================================
  //============= private variables ============================
  //============================================================

  /// Contains the indexes
  std::vector<std::vector<IndexType> > _GhostSendList;
  std::vector<std::vector<IndexType> > _GhostReceiveList;

  /// The real size of an element stored in the vector
  /// (as opposed to the size of the element TYPE)
  size_t _ElementSize;

  /// The number of local owned points
  IndexType _LocalSize;

  /// The number of Ghost points
  IndexType _GhostSize;

  /// The index of the next free element in the vector
  /// (can be NO_MORE_FREE)
  IndexType _NextFree;

  /// This stores the actual element data
  COOLFluiD::Common::ArrayAllocator<T> _Data;

  /// This stores the metadata for an element
  COOLFluiD::Common::ArrayAllocator<DataType> _MetaData;

  /// Do we have an index?
  bool    _IsIndexed;

  /// Some constants... They are the same for every
  /// instantiation; TODO: Try out some scheme with extern
  /// to avoid duplication of useless constants
  static const IndexType _NO_MORE_FREE;
  static const IndexType _FLAG_DELETED;
  static const IndexType _FLAG_GHOST;
  static const IndexType _NOT_FOUND;

  /// The used tags
  /// TODO: Check out interference from multiple vectors
  /// Solution1:
  ///    * Limit to at most one par_vector in each communicator
  /// Solution2:
  ///    * Have it request a free tag from some allocator
  ///      (WARNING! CONSISTENCY IN PARALLEL RUN!)
  static const int _MPI_TAG_BUILDGHOSTMAP;
  static const int _MPI_TAG_SYNC;

  /// The rank of this CPU (cached for speed reasons)
  int _CommRank;

  /// The size of the communicator (cached for speed reasons)
  int _CommSize;

  /// Check to see if InitMPI was called
  bool _InitMPIOK;

  /// Is the CGlobalMap is valid
  bool _CGlobalValid;

  /// The used Communicator
  MPI_Comm _Communicator;

  /// The Index for ghost points
  TGhostMap _GhostMap;

  /// The index for local points
  TIndexMap _IndexMap;

  /// Array containing for each rank the type to send
  /// (can be in seperate subclass)
  std::vector<MPI_Datatype> _SendTypes;

  /// Array containing for each rank the type to receive
  /// (can be in seperate subclass)
  std::vector<MPI_Datatype> _ReceiveTypes;

  /// This is the MPI type of 1 element
  MPI_Datatype _BasicType;

  /// To track the requests
  std::vector<MPI_Request> _ReceiveRequests;
  std::vector<MPI_Request> _SendRequests;

  /// Data of the CGLobal map
  std::vector<IndexType> _CGlobal;

  /// The CGlobal index of our first local element
  IndexType _FirstCGlobal;

  //=============================================================
  //=================== Private Functions =======================
  //=============================================================

  /// Allocate a free element and return the index
  IndexType AllocNext ();

  /// Enlarge the capacity of the vector by GrowBy
  /// If GrowBy is 0, select the optimal enlargment
  void Grow (IndexType GrowBy = 0);

  /// Remove overcapacity
  void Shrink ();


  /// Ghost map builder help routines
  void Sync_BroadcastNeeded ();
  void Sync_BuildSendTypes ();
  void Sync_BuildReceiveList ();
  void Sync_BuildReceiveTypes ();

  void Sync_BuildTypeHelper (const std::vector<std::vector<IndexType> > & V,
                                   std::vector<MPI_Datatype> & MPIType ) const;

  /// Find functions (for internal use)
  /// These take advantage of a index map if one is present
  IndexType FindLocal (IndexType GlobalIndex) const;
  IndexType FindGhost (IndexType GlobalIndex) const;

  void AddLocalIndex (IndexType LocalIndex, IndexType GlobalIndex);
  void AddGhostIndex (IndexType LocalIndex, IndexType GlobalIndex);

  IndexType NormalIndex (IndexType GlobalIndex) const;

  //= Flag helper functions
  inline bool IsFlagSet (IndexType GlobalIndex, IndexType Fl) const;
  inline IndexType SetFlag (IndexType GlobalIndex, IndexType Fl) const;
  inline IndexType ClearFlag (IndexType GLobalIndex, IndexType Fl) const;

#ifdef CF_ENABLE_PARALLEL_DEBUG
  void WriteCommPattern () const;
#endif

public: // funcions

  /// Constructor.
  /// WARNING: Size parameter is IGNORED!
  ParVectorMPI (const T& Init, CF::Uint Size, CF::Uint ESize = 0);

  /// Destructor
  /// Before destructing the class, DoneMPI should be called
  ~ParVectorMPI ();

  /// Returns the list of ghost nodes (by processor rank) to be sent to
  /// another processor
  const std::vector< std::vector< IndexType > >& GetGhostSendList() const
  {
    return _GhostSendList;
  }

  /// Returns the list of ghost nodes (by processor rank) to be received
  /// from another processor
  const std::vector< std::vector< IndexType > >& GetGhostReceiveList() const
  {
    return _GhostReceiveList;
  }

  /// Return the total vector size (not counting ghost points)
  /// This is a COLLECTIVE operation!
  IndexType GetGlobalSize () const; /* collective */

  /// Return the number of local (non-ghost) points
  /// Local operation.
  IndexType GetLocalSize () const; /* local */

  /// return the number of ghostpoints
  /// Local operation.
  IndexType GetGhostSize () const; /* local */

  /// For valarray compatibility;
  /// Return the local size: This is the number of
  /// locally owned points incremented by the number of ghost points
  /// Local operation.
  IndexType size () const;

  /// Returns the total local size (same as size())
  /// Local operation.
  IndexType GetTotalSize () const;

  /// Insert a new ghost point
  /// Local operation.
  /// For now, NO Add operations are allowed after
  /// BuildGhostMap is called.
  IndexType AddGhostPoint (IndexType GlobalIndex);

  /// Insert new local point
  /// Local operation.
  /// For now, NO add operations are allowed after
  /// BuildGhostMap is called.
  IndexType AddLocalPoint (IndexType GlobalIndex);

  /// Local to global mapping:
  ///  (LOCAL operation)
  /// To determine if an element is a ghost element,
  /// use IsGhost ()
  IndexType LocalToGlobal (IndexType LocalIndex) const; /* local */

  /// Global to local mapping.
  /// Can be slow if no indexes were built.
  IndexType GlobalToLocal (IndexType GlobalIndex) const; /* local */

  /// Start the synchronisation
  /// Collective operation.
  /// Before this can be called, InitMPI had to be called.
  void BeginSync ();

  /// Wait for the end of the synchronisation
  /// Collective.
  void EndSync ();

  /// Build internal data structures
  /// (to be called after adding ghost points but before  )
  /// (doing a sync                                       )
  /// Collective.
  /// InitMPI needs to be called before this.
  void BuildGhostMap ();

  /// Create the indexes
  /// (to speed up index operations)
  /// (An index counter is maintained)
  void CreateIndex ();

  /// Free the indexes (to conserve memory)
  void DestroyIndex ();


  /// Given a pointer, try to find the index of the element
  /// (HACK ! DON'T USE)
  IndexType PointerToIndex (const void * Ptr) const;

  /// Write some internal state information
  void DumpInternalData ();

  /// print local+global index to a dumpfile
  void DumpContents ();


  /// Initialize MPI functions
  void InitMPI ();

  /// Free MPI resources
  void DoneMPI ();

  /// Element access (prefered method)
  /// Never take the memory location of an element as it
  /// CAN and WILL change on resizing)
  /// [We do it because of the state mess]
  inline const T & operator [] (IndexType Index) const;

  /// Element access (prefered method)
  /// Never take the memory location of an element as it
  /// CAN and WILL change on resizing)
  /// [We do it because of the state mess]
  inline T & operator [] (IndexType Index);


  /// Make sure we can have up to capacity elements
  /// before needing to allocate
  /// (and possible invalidate pointers & references)
  void reserve (IndexType capacity, CF::Uint elementSize);

  /// Returns true if the given local index is a ghost element
  inline bool IsGhost (IndexType LocalID) const;

  //=========================================================================
  //===== Global Continuous Indexes =========================================
  //=========================================================================

  /// Build a continuous global mapping.
  /// Should be called after all points are added,
  /// and after BuildMap is called
  /// This function uses indexes if available
  void BuildCGlobal ();

  /// returns true if a global continuous mapping is available
  bool HasCGlobal () const;

  /// free the continuous index
  void FreeCGlobal ();

  /// Lookup the global continuous ID of a local element
  inline IndexType LocalToCGlobal (IndexType LocalID) const;

protected:

  /// (Used by internal functions)
  /// This function is used to inform that the CGlobal map is now
  /// invalid because of changes to the local vector size.
  void InvalidateCGlobal ();

  /// Build the CGlobal map for the local elements
  void BuildCGlobalLocal (std::vector<IndexType> & Ghosts);
};

    /// Template Definition
    ///*************************************************************************/


    template <class T>
    inline const T & ParVectorMPI<T>::operator [] (IndexType Index) const
    {
      return _Data[Index];
    }

    template <class T>
    inline T & ParVectorMPI<T>::operator [] (IndexType Index)
    {
      return _Data[Index];
    }

    ///  Static constants
    ///*************************************************************************/

    //
    // MPI TAGS
    //
    template <class T>
    const int ParVectorMPI<T>::_MPI_TAG_BUILDGHOSTMAP  = 100;

    template <class T>
    const int ParVectorMPI<T>::_MPI_TAG_SYNC  =
      ParVectorMPI<T>::_MPI_TAG_BUILDGHOSTMAP + 1;


    //
    // Index flags
    //
    template <class T>
    const typename ParVectorMPI<T>::IndexType ParVectorMPI<T>::_NO_MORE_FREE =
      std::numeric_limits<typename ParVectorMPI<T>::IndexType>::max();

    template <class T>
    const typename ParVectorMPI<T>::IndexType ParVectorMPI<T>::_FLAG_DELETED =
      (std::numeric_limits<IndexType>::max()/ 2) + 1;

    template <class T>
    const typename ParVectorMPI<T>::IndexType ParVectorMPI<T>::_FLAG_GHOST =
      (ParVectorMPI<T>::_FLAG_DELETED >> 1);

    template <class T>
    const typename ParVectorMPI<T>::IndexType ParVectorMPI<T>::_NOT_FOUND =
      std::numeric_limits<typename ParVectorMPI<T>::IndexType>::max();


    /*============================================================================
      == CGlobal Functions ========================================================
      =============================================================================*/

    template <typename T>
    void ParVectorMPI<T>::BuildCGlobalLocal (std::vector<IndexType> & Ghosts)
    {
      // Ghosts contains the local index of the ghost element

      // The number of locally owned elements
      const CF::Uint LocalOwned = GetLocalSize();

      CF::Uint StartID;

      // Prefix scan
      MPI_Scan (const_cast<CF::Uint*>(&LocalOwned),
          &StartID, 1, MPI_UNSIGNED,
          MPI_SUM, _Communicator);

      // Correct for exclusive scan
      StartID -= LocalOwned;

      _FirstCGlobal = StartID;
      CFLogDebug( "First CGlobal ID: " << _FirstCGlobal <<
         ", Count=" << LocalOwned << "\n");

      // resize CGlobal
      _CGlobal.resize (GetLocalSize()+GetGhostSize());

      cf_assert (GetTotalSize()==GetLocalSize()+GetGhostSize());

      for (CF::Uint i=0; i<GetTotalSize(); ++i)
        {
    if (IsGhost (i))
      {
        // Ghosts holds the local indexes of the ghost elements
        // (the global one can be found directly)
        Ghosts.push_back (i);
        continue;
      }

    _CGlobal[i] = StartID;

    ++StartID;
        }

      CFLogDebug( "ParVector: CMap: Remaining to translate: "
         << Ghosts.size() << " ghost elements...\n");
      // Now all CGlobal IDs are set for every non-ghost element
    }

    template <typename T>
    bool ParVectorMPI<T>::HasCGlobal () const
    {
      return _CGlobalValid;
    }



    template <typename T>
    void ParVectorMPI<T>::BuildCGlobal ()
    {
      if (!_IsIndexed) {
        CreateIndex();
      }

      // Determine maximum number of ghost elements
      const CF::Uint LocalGhostSize = GetGhostSize();
      CF::Uint MaxGhostSize = 0;

      Common::CheckMPIStatus(MPI_Allreduce (const_cast<CF::Uint *>(&LocalGhostSize),
            &MaxGhostSize, 1,
            MPI_UNSIGNED, MPI_MAX, _Communicator));

      // Create a vector to do the translation for every ghost
      std::vector<IndexType> Ghosts;
      Ghosts.reserve (LocalGhostSize);

      // Fill local map
      BuildCGlobalLocal (Ghosts);

      // Now Ghosts has all the local indexes of our ghost elements

      cf_assert (Ghosts.size () == GetGhostSize());

      //
      // Here we ask all other CPU's to help translating our
      // ghost ID's to global continuous IDs
      //

      // Find out neigbours
      // TODO RING communicator

      int SendTo = (_CommRank == _CommSize - 1 ? 0 : _CommRank + 1);
      int ReceiveFrom = (_CommRank ? _CommRank - 1 : _CommSize - 1);

      CFLogDebug( "Starting ghost element lookup..."
         " Receiving from " << ReceiveFrom << ", sending to "
         << SendTo << "\n");

      MPI_Request SendRequest;
      MPI_Request ReceiveRequest;


      std::vector<IndexType> SendBuf (MaxGhostSize+1, 666);
      std::vector<IndexType> ReceiveBuf (MaxGhostSize+1, 666);

      // We store all the global IDs we want to translate in the
      // send buffer (first element is number of elements following)
      // and set the ghost flag to indicate that the number isn't translated
      // yet
      SendBuf[0] = Ghosts.size();
      for (CF::Uint i=0; i<static_cast<CF::Uint>(Ghosts.size()); ++i)
        SendBuf[i+1] = SetFlag(LocalToGlobal(Ghosts[i]), _FLAG_GHOST);

      for (CF::Uint Round = 0; Round < static_cast<CF::Uint>(_CommSize);
    ++Round)
        {
    Common::CheckMPIStatus(MPI_Isend (&SendBuf[0], SendBuf[0]+1,
            DataTypeHandler::GetType<IndexType>(),
            SendTo, Round, _Communicator, &SendRequest));
    Common::CheckMPIStatus(MPI_Irecv (&ReceiveBuf[0], ReceiveBuf.size(),
            DataTypeHandler::GetType<IndexType>(),
            ReceiveFrom, Round, _Communicator, &ReceiveRequest));

    Common::CheckMPIStatus(MPI_Wait (&ReceiveRequest, MPI_STATUS_IGNORE));

    // Process receive buffer

    // It is impossible that we receive more elements than we agreed on.
    const CF::Uint EleCount = ReceiveBuf[0];

    cf_assert (EleCount <= MaxGhostSize);

    CFLogDebug( "BuildCMap: Translating "
             << EleCount << " elements...\n");

    for (CF::Uint i=0; i<EleCount; ++i)
      {
        const CF::Uint CurID = i+1;

        const IndexType CurVal = ReceiveBuf[CurID];

        // We have to map CurGlobalID to CGlobalID

        // If it is already mapped, skip
        if (!IsFlagSet(CurVal, _FLAG_GHOST))
          {
      CFLogDebugMed( "Skipping " <<
               NormalIndex(CurVal) << "\n");
      continue;
          }

        // We check if we have it
        IndexType LocalID = FindLocal (NormalIndex(CurVal));

        // If we don't have it, leave it
        if (LocalID == _NOT_FOUND)
          {
      CFLogDebugMed( "Don't have " << NormalIndex(CurVal)
               << "\n");
      continue;
          }

        // Now we have the local ID overhere, replace the
        // global one with the CGlobal one

        // For non-ghost entries CGlobal is already valid...

        cf_assert (!IsGhost (LocalID));

        CFLogDebugMed( "Translating "
           << NormalIndex (CurVal) <<
           " by " << _CGlobal[LocalID] << "\n");

        ReceiveBuf[CurID] = _CGlobal[LocalID];
      }

    Common::CheckMPIStatus(MPI_Wait (&SendRequest, MPI_STATUS_IGNORE));

    SendBuf.swap (ReceiveBuf);
        }

      // After _CommSize rounds, we have back our own values
      // in SendBuf
      cf_assert (SendBuf.size()>=SendBuf[0]+1);
      cf_assert (SendBuf[0] == GetGhostSize ());
      cf_assert (Ghosts.size() == GetGhostSize ());

      // Now we have the global mapping ghosts and the translated
      // mapping in SendBuf...
      for (CF::Uint i=0; i<Ghosts.size(); ++i)
        {
    _CGlobal[Ghosts[i]] = SendBuf[i+1];
    cf_assert (!IsFlagSet (SendBuf[i+1], _FLAG_GHOST));
        }

      _CGlobalValid = true;
    }

    template <typename T>
    void ParVectorMPI<T>::FreeCGlobal ()
    {
      if (_CGlobalValid)
        InvalidateCGlobal ();
    }

    template <typename T>
    void ParVectorMPI<T>::InvalidateCGlobal ()
    {
      _CGlobalValid = false;
      std::swap (_CGlobal, std::vector<CF::Uint>());
    }

    template <typename T>
    typename ParVectorMPI<T>::IndexType ParVectorMPI<T>::LocalToCGlobal (IndexType LocalID) const
    {
      cf_assert (_CGlobalValid);
      cf_assert (LocalID < _CGlobal.size());
      // always true // cf_assert (LocalID >= 0);
      return _CGlobal[LocalID];
    }


    /*============================================================================
    /// Flag functions
    ///////////////////////////////////////////////////////////////////////////////=*/


    template <class T>
    inline typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::ClearFlag (IndexType Global, IndexType Flag) const
    {
      return (Global & (~Flag));
    }

    template <class T>
    inline typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::SetFlag (IndexType Global, IndexType Flag) const
    {
      return (Global | (Flag));
    }

    template <class T>
    inline bool ParVectorMPI<T>::IsFlagSet (IndexType Global,
                IndexType Flag) const
    {
      return (Global & Flag);
    }


    /* ==================================================
    ///  Index functions
    /// ==================================================*/
    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::PointerToIndex (const void * Ptr) const
    {
      IndexType Index = _Data.PointerToIndex (Ptr);
      if (Index==IndexType(-1))
        return _NO_MORE_FREE;

      return Index;
    }

    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::NormalIndex (IndexType Global) const
    {
      return ClearFlag (Global, _FLAG_DELETED|_FLAG_GHOST);
    }

    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::FindLocal (IndexType GlobalIndex) const
    {
      cf_assert (!IsFlagSet (GlobalIndex, _FLAG_GHOST|_FLAG_DELETED));

      if (_IsIndexed)
        {
    typename TIndexMap::const_iterator Iter = _IndexMap.find(GlobalIndex);
    if (Iter == _IndexMap.end ())
      return _NOT_FOUND;

    return Iter->second;
        }

      CFLogDebugMax( "Warning: Using slow FindLocal (no index created) in"
         " parvector\n");
      for (CF::Uint i=0; i<size (); i++)
        {
    IndexType I = _MetaData[i].GlobalIndex;
    if (IsFlagSet (I, _FLAG_DELETED|_FLAG_GHOST))
      continue;
    if (NormalIndex(I)==GlobalIndex)
      return i;
        }
      return _NOT_FOUND;
    }

    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::FindGhost (IndexType GlobalIndex) const
    {
      cf_assert (!IsFlagSet (GlobalIndex, _FLAG_GHOST|_FLAG_DELETED));

      if (_IsIndexed)
        {
    typename TGhostMap::const_iterator Iter = _GhostMap.find(GlobalIndex);
    if (Iter == _GhostMap.end ())
      return _NOT_FOUND;
    return Iter->second;
        }

      CFLogDebugMax( "Warning: Using slow FindGhost (no index created) in"
         " parvector\n");
      for (CF::Uint i=0; i<size (); i++)
        {
    IndexType I = _MetaData[i].GlobalIndex;
    if (!IsFlagSet (I, _FLAG_GHOST))
      continue;
    if (IsFlagSet (I, _FLAG_DELETED))
      continue;
    if (NormalIndex(I)==GlobalIndex)
      return i;
        }
      return _NOT_FOUND;
    }

    template <class T>
    void ParVectorMPI<T>::DestroyIndex ()
    {
      cf_assert (_IsIndexed);

      _IndexMap.clear();
      _GhostMap.clear();

      _IsIndexed = false;
    }

    template <class T>
    void ParVectorMPI<T>::AddLocalIndex (IndexType Local, IndexType Global)
    {
      if (!_IsIndexed)
        return;
      cf_assert (!IsFlagSet (Global, _FLAG_DELETED|_FLAG_GHOST));
      cf_assert (_IndexMap.find(Global)==_IndexMap.end());

      _IndexMap[Global]=Local;
    }

    template <class T>
    void ParVectorMPI<T>::AddGhostIndex (IndexType Local, IndexType Global)
    {
      if (!_IsIndexed)
        return;

      cf_assert (_GhostMap.find(Global)==_GhostMap.end());

      _GhostMap[Global]=Local;
    }

    template <class T>
    void ParVectorMPI<T>::CreateIndex ()
    {
      cf_assert (!_IsIndexed);

      // Also erases memory
      _IndexMap.clear ();
      _GhostMap.clear ();

      for (unsigned int i=0; i < _Data.GetSize (); ++i)
      {
    IndexType Global = _MetaData[i].GlobalIndex;

    if (IsFlagSet (Global, _FLAG_DELETED))
      continue;

    if (IsFlagSet (Global, _FLAG_GHOST))
      _GhostMap[NormalIndex(Global)]=i;
    else
      _IndexMap[NormalIndex(Global)]=i;
        }

      cf_assert (_GhostMap.size ()==_GhostSize);
      cf_assert (_IndexMap.size ()==_LocalSize);

      _IsIndexed=true;
    }


    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::LocalToGlobal (IndexType LocalIndex) const
    {
      return NormalIndex(_MetaData[LocalIndex].GlobalIndex);
    }


    /// Return true if the element with local ID LocaLID is a ghost element
    template <typename T>
    inline bool ParVectorMPI<T>::IsGhost (IndexType LocalID) const
    {
      return IsFlagSet (_MetaData[LocalID].GlobalIndex, _FLAG_GHOST);
    }


    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::GlobalToLocal (IndexType GlobalIndex) const
    {

      // TODO: adapt for search functions
      //
      typename TIndexMap::const_iterator Iter = _IndexMap.find(GlobalIndex);
      if (Iter!=_IndexMap.end())
        return Iter->second;

      typename TGhostMap::const_iterator Iter2 = _GhostMap.find(GlobalIndex);
      if (Iter2!=_GhostMap.end())
        return Iter2->second;

      throw ParallelError (FromHere(),"Parvector: local index not found");
    }


    /*============================================================
    *  Building synchronisation MPI types
    *============================================================*/
    template <class T>
    void ParVectorMPI<T>::Sync_BuildReceiveTypes ()
    {
      Sync_BuildTypeHelper (_GhostReceiveList, _ReceiveTypes);
    }


    template <class T>
    void ParVectorMPI<T>::Sync_BroadcastNeeded ()
    {
      IndexType MaxGhostSize;

      // Determine needed buffer size
      Common::CheckMPIStatus (MPI_Allreduce (&_GhostSize, &MaxGhostSize, 1, MPI_INT, MPI_MAX,
             _Communicator));

      // it is unsigned, so this is useless // cf_assert(MaxGhostSize>=0);

      if (!MaxGhostSize)
        return;     // No node has ghost points

      // Allocate storage
      const IndexType StorageSize = MaxGhostSize+1;
      IndexType * Storage = new IndexType[StorageSize];

      typename TIndexMap::const_iterator Iter;

      // Broadcast needed points
      for (int RankTurn=0; RankTurn<_CommSize; RankTurn++)
      {
	  if (RankTurn==_CommRank)
	  {
	      //
	      // We send our list
	      //
	      Storage[0]=_GhostMap.size();

	      int i=1;
	      for (typename TGhostMap::const_iterator Iter=_GhostMap.begin();
		   Iter!=_GhostMap.end(); Iter++)
		  Storage[i++]=Iter->first;

	      Common::CheckMPIStatus(MPI_Bcast (Storage, StorageSize, MPI_INT, RankTurn,
						_Communicator));
	  }
	  else
	  {
	      //
	      // Time to receive the list
	      //
	      Common::CheckMPIStatus(MPI_Bcast (Storage, StorageSize, MPI_INT, RankTurn,
						_Communicator));

	      const IndexType Aantal = Storage[0];
	      cf_assert (Aantal <= MaxGhostSize);

	      for (IndexType j=1; j<=Aantal; j++)
	      {
		  // Could use GlobalToLocal here, the exception-
		  // overhead would be too big.
		  Iter=_IndexMap.find( Storage[j]);

		  if (Iter==_IndexMap.end())
		      continue; // We don't have this one

		  _GhostSendList[RankTurn].push_back(Iter->second);
	      }
	  }

	  //MPI_Barrier (_Communicator);
      }

      delete[] (Storage);
    }

    template <class T>
    void ParVectorMPI<T>::Sync_BuildReceiveList ()
    {
      IndexType MaxSendSize = 0;

      // Allocate bufferspace
      for (IndexType i=0; i< (IndexType) _CommSize; i++)
        MaxSendSize = std::max(
             MaxSendSize, static_cast<IndexType>(_GhostSendList[i].size()) );

      IndexType * ReceiveStorage = new IndexType[_CommSize*_GhostSize];
      IndexType * SendStorage = new IndexType[MaxSendSize];
      MPI_Request * Requests = new MPI_Request[_CommSize];

      // Post receives
      for (int i=0; i<_CommSize; i++)
        {
    if (i==_CommRank)
      {
        Requests[i]=MPI_REQUEST_NULL;
        continue;
      }

    Common::CheckMPIStatus(MPI_Irecv (&ReceiveStorage[i*_GhostSize], _GhostSize,
            MPI_INT, i, _MPI_TAG_BUILDGHOSTMAP, _Communicator,
            &Requests[i]));
        }

      // Send ghost points
      for (int i=0; i<_CommSize; i++)
        {
    if (i==_CommRank)
      continue;

    IndexType j=0;
    for (typename std::vector<IndexType>::const_iterator iter=_GhostSendList[i].begin();
         iter!=_GhostSendList[i].end (); iter++)
      SendStorage[j++]=NormalIndex(_MetaData[*iter].GlobalIndex);

    Common::CheckMPIStatus(MPI_Send (SendStorage, _GhostSendList[i].size(),
           DataTypeHandler::GetType<IndexType>(), i,
           _MPI_TAG_BUILDGHOSTMAP, _Communicator));
        }

      // Wait receives
      while (true)
        {
    int Current;
    MPI_Status Status;

    cf_assert (Requests != CFNULL);

    Common::CheckMPIStatus(MPI_Waitany (_CommSize, Requests, &Current, &Status));

    if (Current==MPI_UNDEFINED)
      break;

    cf_assert (Requests[Current]==MPI_REQUEST_NULL);

    IndexType Aantal;

    MPI_Get_count (&Status, DataTypeHandler::GetType<IndexType>(),
             (int *) &Aantal);

    // it is unsigned, so this is useless // cf_assert (Aantal >= 0 );
    cf_assert(Aantal <= _GhostSize);

    // Fill in receive list
    for (IndexType i=Current*_GhostSize; i<(Current*_GhostSize)+Aantal; i++)
      {
        typename TGhostMap::const_iterator Iter =
          _GhostMap.find(ReceiveStorage[i]);

        cf_assert (Iter!=_GhostMap.end());

        _GhostReceiveList[Current].push_back(Iter->second);
      }
        }

      delete[] (ReceiveStorage);
      delete[] (SendStorage);
      delete[] (Requests);
    }

    template <class T>
    void ParVectorMPI<T>::BuildGhostMap ()
    {
      cf_assert (_InitMPIOK);

      cf_assert (_GhostSendList.size()==
           static_cast<CF::Uint>(_CommSize));
      cf_assert (_GhostReceiveList.size()==
           static_cast<CF::Uint>(_CommSize));

      CFLogInfo("ParVectorMPI<T>::BuildGhostMap() START");

      // Clear old mapping
      for (int j=0; j<_CommSize; j++)
      {
	  _GhostSendList[j].clear();
	  _GhostReceiveList[j].clear();
      }

      // Broadcast needed points
      Sync_BroadcastNeeded ();

      // Build send datatype
      Sync_BuildSendTypes();

      // Now building receive lists
      Sync_BuildReceiveList ();

      // Check if all ghost elements were found...
      IndexType GhostFound = 0;
      for (int i=0; i<_CommSize; i++)
        GhostFound+=_GhostReceiveList[i].size();

      cf_assert (_GhostSize == _GhostMap.size());
      if (GhostFound != _GhostSize)
      {
	  // Error: we don't have all the ghost points
	  CFLog(NORMAL, "Not all ghost points were found! Starting investigation\n");

	  std::set<CF::Uint> Ghosts;
	  std::set<CF::Uint> Receives;
	  std::set<CF::Uint> Missing;

	  typename TGhostMap::const_iterator Iter;

	  for (Iter=_GhostMap.begin(); Iter!=_GhostMap.end(); ++Iter)
	      Ghosts.insert(Iter->first);

	  for (int i=0; i<_CommSize; ++i)
	      std::copy(_GhostReceiveList[i].begin(), _GhostReceiveList[i].end(),
			std::inserter(Receives, Receives.begin()));

	  std::set_difference(Ghosts.begin(), Ghosts.end(), Receives.begin(),
			      Receives.end(), std::inserter(Missing, Missing.begin()));

	  std::ostringstream S;
	  S << "Missing ghost elements (globalID): ";
	  for (std::set<CF::Uint>::const_iterator I = Missing.begin();
	       I!=Missing.end(); ++I)
	      S << *I << " ";
	  S << "\n";

    throw ParallelError (FromHere(), S.str() );
      }

      CFLogInfo("ParVectorMPI<T>::BuildGhostMap()  => Sync_BuildReceiveTypes");

      // Build receive datatype
      Sync_BuildReceiveTypes ();

#ifdef CF_ENABLE_PARALLEL_DEBUG
      WriteCommPattern ();
#endif

      CFLogInfo("ParVectorMPI<T>::BuildGhostMap()  END");
    }

//////////////////////////////////////////////////////////////////////////////


    template <class T>
    void ParVectorMPI<T>::Sync_BuildTypeHelper (const std::vector<std::vector<IndexType> > & V,
              std::vector<MPI_Datatype> & MPIType) const
    {

#ifdef HAVE_MPI_TYPE_GET_TRUE_EXTENT
      // Safety check
      MPI_Aint dummy, extent;
      Common::CheckMPIStatus(MPI_Type_get_true_extent (_BasicType, &dummy,&extent));
      cf_assert ((size_t) extent == ElementSize);
#endif

      for (int i=0; i<_CommSize; i++)
        if (MPIType[i]!=MPI_DATATYPE_NULL)
    MPI_Type_free (&MPIType[i]);


      IndexType MaxSize = 0;
      for (int i=0; i<_CommSize; i++)
        MaxSize = std::max(MaxSize, static_cast<IndexType>(V[i].size ()));

      int * Offset = new int[MaxSize];
      int * Length = new int[MaxSize];

      for (int i=0; i<_CommSize; i++)
        {
    if (!V[i].size())
      continue;

    for (CF::Uint j=0; j<V[i].size(); j++)
      {
        Length[j]=1;
        Offset[j]=V[i][j];
      }

    Common::CheckMPIStatus (MPI_Type_indexed (V[i].size(),Length,Offset,
              _BasicType, &MPIType[i]));
    Common::CheckMPIStatus (MPI_Type_commit (&MPIType[i]));
        }

      delete[] (Offset);
      delete[] (Length);
    }



//////////////////////////////////////////////////////////////////////////////

    template <class T>
    void ParVectorMPI<T>::Sync_BuildSendTypes ()
    {
      Sync_BuildTypeHelper (_GhostSendList, _SendTypes);
    }

//////////////////////////////////////////////////////////////////////////////

    /*==============================================================
    * Allocation of elements
    *==============================================================*/
    template <class T>
    inline void ParVectorMPI<T>::reserve (IndexType reservesize,
              CF::Uint elementSize)
    {
      CFLog ( VERBOSE, "_InitMPIOK  = " << _InitMPIOK << "\n" );
      CFLog ( VERBOSE, "elementSize = " << elementSize << "\n" );
      CFLog ( VERBOSE, "reservesize = " << reservesize << "\n" );

      if (!_InitMPIOK) {
        _Data.cleanup();
        _MetaData.cleanup();
        _Data.initialize(T(), 0, elementSize);
              _MetaData.initialize(DataType(), 0);

        CFLog ( VERBOSE, "Before _ElementSize = " << _ElementSize << "\n" );
        cf_assert(_ElementSize == 0);
        _ElementSize = elementSize;
        CFLog ( VERBOSE, "After _ElementSize = " << _ElementSize << "\n" );
        InitMPI();
      }

      if (reservesize <= _Data.GetSize ())
        return;

      IndexType GrowBy =  reservesize - _Data.GetSize ();
      CFLogDebug( "par_vector reserve called: reservesize="
         << reservesize << ", growing by " << GrowBy
         << ", current size=" << _Data.GetSize() << "\n");
      Grow (GrowBy);
    }

//////////////////////////////////////////////////////////////////////////////


    template <class T>
    void ParVectorMPI<T>::Grow (IndexType GrowBy)
    {
      IndexType OldSize = _Data.GetSize ();
      IndexType NewSize;

      //    cf_assert (GrowBy>=0);

      if (GrowBy)
        _Data.resize (OldSize+GrowBy);
      else
        _Data.Grow ();

      NewSize = _Data.GetSize ();
      _MetaData.Resize(NewSize);

      cf_assert (NewSize>OldSize);
      cf_assert (_MetaData.GetSize() >= _Data.GetSize());

      // Now we have additional storage starting at CurrentSize
      // Walk current free list to prevent fragmentation
      IndexType Current = _NextFree;
      while (Current!=_NO_MORE_FREE &&
      _MetaData[Current].GlobalIndex!=_NO_MORE_FREE)
        Current=_MetaData[Current].GlobalIndex;

      // Now current points to the last free block.

      // Link new free blocks
      for (IndexType i=OldSize; i<(NewSize-1); i++)
        _MetaData[i].GlobalIndex=i+1;
      _MetaData[NewSize-1].GlobalIndex=_NO_MORE_FREE;

      // Only thing left is linking last old free block with the start of the
      // new list
      if (Current!=_NO_MORE_FREE)
        {
    cf_assert(_MetaData[Current].GlobalIndex==_NO_MORE_FREE);
    _MetaData[Current].GlobalIndex=OldSize;
        }

      if (_NextFree==_NO_MORE_FREE)
        _NextFree=OldSize;

      cf_assert (_MetaData.GetSize () >= _Data.GetSize ());
    }



//////////////////////////////////////////////////////////////////////////////


    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::AllocNext ()
    {
      // Check to see if we reached the limit of our base IndexType

      if (IsFlagSet (size()+1, _FLAG_DELETED|_FLAG_GHOST))
        {
    CFLog(NORMAL, "Limit of IndexType reached!!!!!\n");
    return _NO_MORE_FREE;
        }

      if (_NextFree==_NO_MORE_FREE)
        {
    // Must grow
    Grow ();
        }

      cf_assert (_NextFree!=_NO_MORE_FREE);

      IndexType NewID = _NextFree;
      _NextFree = _MetaData[NewID].GlobalIndex; // From free list


      return NewID;
    }



//////////////////////////////////////////////////////////////////////////////


    /// Dump the local contents
    /// (non-global operation)
    /// Can be called before buildmap was called
    template <class T>
    void ParVectorMPI<T>::DumpContents ()
    {
      std::ostringstream S;
      S << "parvector_cont.dump." << _CommRank;

      std::ofstream Out (S.str().c_str());

      Out << "Content dump for rank " << _CommRank << "\n";

      for (CF::Uint i=0; i<size(); ++i)
        {
    int Global = LocalToGlobal(i);
    Out << i << " " << Global;
    if (IsFlagSet (_MetaData[i].GlobalIndex, _FLAG_GHOST))
      Out << " [ghost]";
    Out << "\n";
        }
      Out << "Size = " << size() ;
      Out << " [_GhostSize = " << _GhostSize ;
      Out << ", _LocalSize = " << _LocalSize << "]\n";
    }



//////////////////////////////////////////////////////////////////////////////


    template <class T>
    void ParVectorMPI<T>::DumpInternalData ()
    {
      std::ostringstream S;
      S << "parvector.dump." << _CommRank;

      std::ofstream Out (S.str().c_str());
      MPI_Barrier(_Communicator);

      if (!_CommRank)
        {
    Out << "Flags : \n";
    Out << " _NO_MORE_FREE: " << _NO_MORE_FREE <<"\n";
    Out << " _FLAG_DELETED: " << _FLAG_DELETED << "\n";
    Out << " _FLAG_GHOST:   " << _FLAG_GHOST << "\n";
    Out << "\n";
        }

      for (int j=0; j<_CommSize; j++)
        {
    if (j==_CommRank)
      {
        Out << "Ghost map for node " << _CommRank << "\n";
        Out <<  "--------------------" << "\n";
        // Write out ghost map
        for (int i=0; i<_CommSize; i++)
          {
      Out << _CommRank << ": Ghost send list to node " << i << ": ";
      for (typename std::vector<IndexType>::const_iterator iter=_GhostSendList[i].begin();
           iter!=_GhostSendList[i].end(); iter++)
        Out <<  *iter << "(" <<
          NormalIndex(_MetaData[*iter].GlobalIndex) << ") ";
      Out << "Receive: ";
      for (typename std::vector<IndexType>::const_iterator
             iter=_GhostReceiveList[i].begin();
           iter!=_GhostReceiveList[i].end(); iter++)
        {
          Out << *iter << "(" <<
            NormalIndex(_MetaData[*iter].GlobalIndex) << ") ";
        }

      Out <<  "\n";
          }
        Out <<  "Indexmap: ";
        for (typename TIndexMap::const_iterator Iter=_IndexMap.begin(); Iter!=_IndexMap.end(); Iter++)
          {
      Out << Iter->first << " " ;
          }
        Out << "\n\n";
      }
    MPI_Barrier(_Communicator);
        }
    }


//////////////////////////////////////////////////////////////////////////////



    //
    //* Syncs ghostpoints
      //
    template <class T>
    void ParVectorMPI<T>::BeginSync ()
    {
      cf_assert (_InitMPIOK);

      //
      // TODO: dit kan beter
      //   Onnodig om over de hele lijst te lopen
      //   (anders niet schaalbaar in functie v/ aantal nodes)
      //

      // Post receives
      for (int i=0; i<_CommSize; i++)
        {
    if (i==_CommRank)
      continue;

    //
    // Idea: use persistent requests
    // (try to measure performance improvement)
    //
    if (!_GhostReceiveList[i].empty())
      {
        Common::CheckMPIStatus(MPI_Irecv (&_Data[0], 1, _ReceiveTypes[i], i,
                _MPI_TAG_SYNC, _Communicator, &_ReceiveRequests[i]));
      }

    if (!_GhostSendList[i].empty())
      {
        Common::CheckMPIStatus(MPI_Isend (&_Data[0], 1, _SendTypes[i], i, _MPI_TAG_SYNC,
                _Communicator, &_SendRequests[i]));
      }
        }
    }


//////////////////////////////////////////////////////////////////////////////



    template <class T>
    void ParVectorMPI<T>::EndSync ()
    {
      cf_assert (_InitMPIOK);

      // In feite is volgende niet nodig aangezien receives niet kunnen
      // klaar zijn alvorens de sends klaar zijn

      // Misschien 1 grote array gebruiken om 1 MPI_Waitall te kunnen doen
      Common::CheckMPIStatus(MPI_Waitall (_CommSize, &_SendRequests[0], MPI_STATUSES_IGNORE));
      Common::CheckMPIStParallelErroratus(MPI_Waitall (_CommSize, &_ReceiveRequests[0], MPI_STATUSES_IGNORE));
    }


//////////////////////////////////////////////////////////////////////////////



    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::AddGhostPoint (IndexType GlobalIndex)
    {
      typename TGhostMap::const_iterator Iter = _GhostMap.find(GlobalIndex);

      if (Iter!=_GhostMap.end())
        throw ParallelError (FromHere(), "Parvector: AddGhostPoint: didnt find global index");

      //
      // Alternative:
      //    return index for the local point if a ghost point for a
      //    local point is requested
      //
      typename TIndexMap::const_iterator Iter2 = _IndexMap.find(GlobalIndex);
      if (Iter2!=_IndexMap.end())
        throw ParallelError (FromHere(), "ParVector: AddGhostPoint: inserting double element");

      IndexType NewLocalID = AllocNext ();

      cf_assert (NewLocalID!=_NO_MORE_FREE);
      //  cf_assert (GlobalIndex>=0);

      _MetaData[NewLocalID].GlobalIndex = SetFlag(GlobalIndex, _FLAG_GHOST);
      _GhostSize++;

      _GhostMap[GlobalIndex]=NewLocalID;

      CFLogDebugMax( "AddGhostPoint: local=" << NewLocalID << ", global=" << GlobalIndex << "\n");

      return NewLocalID;
    }


//////////////////////////////////////////////////////////////////////////////



    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::AddLocalPoint (IndexType GlobalIndex)
    {
      IndexType NewLocalID = AllocNext ();

      if (NewLocalID == _NO_MORE_FREE )
        throw ParallelError (FromHere(), "ParVector: AddLocalPoint: No more free space");

      cf_assert (NewLocalID!=_NO_MORE_FREE);

      _MetaData[NewLocalID].GlobalIndex = ClearFlag(GlobalIndex, _FLAG_GHOST);
      _LocalSize++;

      typename TIndexMap::const_iterator Iter = _IndexMap.find(GlobalIndex);
      if (Iter != _IndexMap.end())
        throw ParallelError (FromHere(), "ParVector: AddLocalPoint: inserting double element");

      _IndexMap[GlobalIndex]=NewLocalID;


      CFLogDebugMax( "Add localpoint: local " << NewLocalID << ", global " << GlobalIndex << "\n");

      return NewLocalID;
    }



//////////////////////////////////////////////////////////////////////////////



    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::size () const
    {
      return GetLocalSize () + GetGhostSize ();
    }


//////////////////////////////////////////////////////////////////////////////



    template <typename T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::GetTotalSize () const
    {
      return size();
    }


//////////////////////////////////////////////////////////////////////////////



    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::GetGhostSize () const
    {
      cf_assert (_GhostMap.size()==_GhostSize);
      return _GhostSize;
    }


//////////////////////////////////////////////////////////////////////////////



    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::GetLocalSize () const
    {
      cf_assert (_IndexMap.size()<=_LocalSize);
      return _LocalSize;
    }


//////////////////////////////////////////////////////////////////////////////



    template <class T>
    typename ParVectorMPI<T>::IndexType
    ParVectorMPI<T>::GetGlobalSize () const
    {
      cf_assert (_InitMPIOK);

      IndexType Total = 0;
      IndexType Local = GetLocalSize();

      Common::CheckMPIStatus(MPI_Allreduce (&Local, &Total, 1,
            DataTypeHandler::GetType<IndexType>(), MPI_SUM,
            _Communicator));

      return Total;
    }


//////////////////////////////////////////////////////////////////////////////

    template <class T>
    void ParVectorMPI<T>::DoneMPI ()
    {
#ifndef NDEBUG
      cf_assert (_InitMPIOK == true);
      _InitMPIOK = false;
#endif
      //    MPI_Waitall (_CommSize, _ReceiveRequests, MPI_STATUSES_IGNORE);
      //    MPI_Waitall (_CommSize, _ReceiveRequests, MPI_STATUSES_IGNORE);

      for (int i = 0; i <_CommSize; i++) {
       	if (_SendTypes[i]!=MPI_DATATYPE_NULL) {
	  MPI_Type_free (&_SendTypes[i]);
	}
	if (_ReceiveTypes[i]!=MPI_DATATYPE_NULL) {
	  MPI_Type_free (&_ReceiveTypes[i]);
	}
      }

      CFLogDebug( "parVector DoneMPI\n");
    }


//////////////////////////////////////////////////////////////////////////////


    template <class T>
    void ParVectorMPI<T>::InitMPI ()
    {
      //TODO: set errhandler: MPI_Comm_set_errhandler / MPI_Errhandler_set
      //
      // No need for error checking, default MPI error handling = abort
      //
      // get the communicator
      _Communicator = PE::GetPE().GetCommunicator();

      cf_assert (_InitMPIOK == false);
      _InitMPIOK = true;

      MPI_Comm_rank (_Communicator, &_CommRank);
      MPI_Comm_size (_Communicator, &_CommSize);

      _GhostSendList.resize (_CommSize);
      _GhostReceiveList.resize (_CommSize);
      _SendTypes.resize (_CommSize);
      _ReceiveTypes.resize (_CommSize);
      _ReceiveRequests.resize (_CommSize);
      _SendRequests.resize (_CommSize);

      for (int i=0; i<_CommSize; i++)
        {
    _SendTypes[i]=_ReceiveTypes[i]=MPI_DATATYPE_NULL;
    _ReceiveRequests[i]=MPI_REQUEST_NULL;
    _SendRequests[i]=MPI_REQUEST_NULL;
        }

      // Need to set the basic type
      if (_ElementSize != sizeof (T))
        {
    // We have to provide our own...
    // TODO: !!! this can cause trouble !!!
    MPI_Type_contiguous (_ElementSize, MPI_BYTE, &_BasicType);
    MPI_Type_commit (&_BasicType);
        }
      else
        {
#ifdef HAVE_MPI_TYPE_DUP
    MPI_Type_dup (DataTypeHandler::GetType<T>(), &_BasicType);
    MPI_Type_commit (&_BasicType);
#else
    _BasicType = DataTypeHandler::GetType<T>();
#endif
        }

      CFLogDebug( "parvector InitMPI\n");
    }

//////////////////////////////////////////////////////////////////////////////

    template <class T>
    ParVectorMPI<T>::~ParVectorMPI ()
    {
      DoneMPI();
    }


//////////////////////////////////////////////////////////////////////////////

    template <class T>
    ParVectorMPI<T>::ParVectorMPI (const T & Init, CF::Uint Size, CF::Uint ESize)
      : _ElementSize(ESize), _LocalSize(0), _GhostSize(0),
        _NextFree(_NO_MORE_FREE), _Data(Init, 0, ESize), _MetaData(DataType(), 0),
        _IsIndexed(false), _InitMPIOK(false), _CGlobalValid(false)
    {
      if (ESize > 0) {
        InitMPI ();
      }
    }

//////////////////////////////////////////////////////////////////////////////

#ifdef CF_ENABLE_PARALLEL_DEBUG
    template <class T>
    void ParVectorMPI<T>::WriteCommPattern () const
    {
      std::vector<CF::Uint> Send(_CommSize), Receive(_CommSize);



      for (CF::Uint i=0; i<_GhostSendList.size(); ++i)
        {
    Send[i] = _GhostSendList[i].size();
    Receive[i] = _GhostReceiveList[i].size();
        }

      WriteCommPatternHelper (_Communicator,
            GetLocalSize(), GetGhostSize(),
            Send, Receive);
    }
#endif

//////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

#endif

