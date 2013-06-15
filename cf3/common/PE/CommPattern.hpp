// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_CommPattern_hpp
#define cf3_common_PE_CommPattern_hpp

#include "common/Component.hpp"
#include "common/BoostArray.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/CommWrapper.hpp"
#include "common/PE/CommWrapperMArray.hpp"

namespace cf3 {
namespace common {
namespace PE {

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file CommPattern.hpp
  @author Tamas Banyai
  @brief Parallel Communication Pattern.
  This class provides functionality to collect communication.
  For efficiency it works such a way that you submit your request via the constructor or the add/remove/move magic triangle and then call setup to modify the commpattern.
  The data needed to be kept synchronous can be registered via the insert function.
  The word node here means any kind of "point of storage", in this context it is not directly related with the computational mesh.
**/

/**
  @todo make possible to use compattern on non-registered data
  @todo when adding, how to give values to the newly createable elements?
  @todo add readonly properties (for example for coordinates, you want to keep it synchronous with commpattern but don't actually want to update it every time)
  @todo propagate CPint through mpiwrapper
  @todo gid registration: must be more straightforward to check if its really a CFuint single stride data
  @todo introduce allocate_component
**/

class Common_API CommPattern: public Component {

public:

  /// @name TYPEDEFS
  //@{

  /// type of integer to use internally in commpattern, to avoid mess of changing type when mpi allows unsigned ints
  typedef int CPint;
  /// typedef for the temporary buffer
  class temp_buffer_item{
    public:
      temp_buffer_item(int _lid, Uint _gid, Uint _rank, bool _option)
      {
        lid=_lid;
        gid=_gid;
        rank=(CPint)_rank;
        option=_option;
      }
      temp_buffer_item()
      {
        lid=std::numeric_limits<int>::max();
        gid=std::numeric_limits<Uint>::max();
        rank=std::numeric_limits<CPint>::max();
        option=false;
      }
      int lid;
      Uint gid;
      CPint rank;
      bool option;
  };
  /// typedef for the temporary buffer array
  typedef std::vector<temp_buffer_item> temp_buffer_array;
  /// enum for flags in dist_struct
  enum dist_struct_flags
  {
    NOFLAGS=0x0,
    GHOST=0x1,
    UPDATABLE=0x2,
    UNUSED=0x4,
    DELETED=0x8,
    ALLDELETE=0x16
  };
  /// helper struct for setup function
  class dist_struct {
    public:
      dist_struct()
      {
        gid=0;
        rank=-1;
        lid=-1;
        data=0;
        flags=UNUSED;
      }
      dist_struct(Uint _gid, CPint _rank, CPint _lid, dist_struct_flags _flags )
      {
        gid=_gid;
        rank=_rank;
        lid=_lid;
        data=0;
        flags=_flags;
      }
      inline bool operator < ( const dist_struct& val ) const { return gid < val.gid;  } // operator std::sort
      Uint  gid;               // global id of the item
      CPint rank;              // rank where the item is updatable
      CPint lid;               // local id on that rank
      void *data;              // packed data if it needs to be moved along procs, otherwise nullptr
      dist_struct_flags flags; // bookkeping flags
  };


  //@} TYPEDEFS

public:

  /// @name CONSTRUCTORS, DESTRUCTORS & OTHER BELONGINGS
  //@{

  /// constructor
  /// @param name under this name will the component be registered
  CommPattern(const std::string& name);

  /// destructor
  ~CommPattern();

  /// Get the class name
  static std::string type_name () { return "CommPattern"; }

  //@} END CONSTRUCTORS/DESTRUCTORS

  /// @name DATA REGISTRATION
  //@{

  /// register data coming from naked pointer by reference
  /// @param name the component will appear under this name
  /// @param data pointer to data
  /// @param size length of the data
  /// @param stride number of array element grouping
  template<typename T> void insert(const std::string& name, T*& data, const int size, const unsigned int stride=1, const bool needs_update=true)
  {
    Handle< CommWrapperPtr<T> > ow = create_component< CommWrapperPtr<T> >(name);
    ow->setup(data,stride,needs_update);
  }

  /// register data coming from pointer to naked pointer
  /// @param name the component will appear under this name
  /// @param data pointer to data
  /// @param size length of the data
  /// @param stride number of array element grouping
  template<typename T> void insert(const std::string& name, T** data, const int size, const unsigned int stride=1, const bool needs_update=true)
  {
    Handle< CommWrapperPtr<T> > ow = create_component< CommWrapperPtr<T> >(name);
    ow->setup(data,stride,needs_update);
  }

  /// register data coming from std::vector by reference
  /// @param name the component will appear under this name
  /// @param pointer to std::vector of data
  /// @param stride number of array element grouping
  template<typename T> void insert(const std::string& name, std::vector<T>& data, const unsigned int stride=1, const bool needs_update=true)
  {
    Handle< CommWrapperVector<T> > ow = create_component< CommWrapperVector<T> >(name);
    ow->setup(data,stride,needs_update);
  }

  /// register data coming from multiarrays by reference
  /// @param name the component will appear under this name
  /// @param data Multiarray holding the data (not copied)
  /// @param stride number of array element grouping
  template<typename ValueT, std::size_t NDims>
  void insert(const std::string& name, boost::multi_array<ValueT, NDims>& data, const bool needs_update=true)
  {
    typedef CommWrapperMArray<ValueT, NDims> CommWrapperT;
    Handle<CommWrapperT> ow = create_component<CommWrapperT>(name);
    ow->setup(data,needs_update);
  }

  /// register data coming from pointer to std::vector
  /// @param name the component will appear under this name
  /// @param pointer to std::vector of data
  /// @param stride number of array element grouping
  template<typename T> void insert(const std::string& name, std::vector<T>* data, const unsigned int stride=1, const bool needs_update=true)
  {
    Handle< CommWrapperVector<T> > ow = create_component< CommWrapperVector<T> >(name);
    ow->setup(data,stride,needs_update);
  }

  /// removes data by name
  void clear( const std::string& name)
  {
    remove_component(name);
    // anything to be deallocated?
  }

  //@} END DATA REGISTRATION

  /// @name COMMPATTERN HANDLING
  //@{

  /// build and/or modify communication pattern - add nodes
  /// this function sets actually up the communication pattern
  /// beware: interprocess communication heavy
  /// this overload of setup is designed for making no callback functions, so all the registered data should match the size of current size + number of additions
  /// @param gid CommWrapper to a Uint tpye of data array
  /// @param rank vector of ranks where given global ids are updatable to add
  void setup(const Handle<CommWrapper>& gid, std::vector<Uint>& rank);

  /// build and/or modify communication pattern - add nodes
  /// this function sets actually up the communication pattern
  /// beware: interprocess communication heavy
  /// this overload of setup is designed for making no callback functions, so all the registered data should match the size of current size + number of additions
  /// @param gid CommWrapper to a Uint tpye of data array
  /// @param rank vector of ranks where given global ids are updatable to add
  void setup(const Handle<CommWrapper>& gid, boost::multi_array<Uint,1>& rank);

  /// build and/or modify communication pattern - only incorporate actual buffers
  /// this function sets actually up the communication pattern
  /// beware: interprocess communication heavy
  void setup();

  /// synchronize the all parallel objects
  void synchronize_all();

  /// synchronize the parallel object designated by its name
  /// @param name the name of the parallel object
  void synchronize( const std::string& name );

  /// synchronize the parallel object designated by its commwrapper reference
  /// @param name the name of the parallel object
  void synchronize( const CommWrapper& pobj );

  /// add element to the commpattern
  /// when all changes done, all needs to be committed by calling setup
  /// if global id is not on current rank, then a ghost is automatically created on current rank
  /// if there is no updatable node on rank, an error is thrown
  /// @param gid global id
  /// @param rank rank where given global node is to be updatable
  /// @see setup for committing changes
  void add_global(Uint gid, Uint rank);

  /// add element to the commpattern
  /// when all changes done, all needs to be committed by calling setup
  /// @param as_ghost decision if node to be kept on from_rank as ghost
  /// @return the local id assigned to the new node. After calling setup, the global id can be looked-up.
  /// @see setup for committing changes
  Uint add_local(bool as_ghost=false);

  /// move element along partitions
  /// when all changes done, all needs to be committed by calling setup
  /// @param lid local id
  /// @param rank rank where the node is expected to be updatable
  /// @param keep_as_ghost decision if node to be kept on from_rank as ghost
  /// @see setup for committing changes
  void move_local(Uint lid, Uint rank, bool keep_as_ghost=true);

  /// delete element from the commpattern
  /// when all changes done, all needs to be committed by calling setup
  /// @param lid local id
  /// @param on_all_ranks force delete on all processes if true. If its false then depends if node is ghost or updatable on rank and deletes only on this rank or all ranks respectively.
  /// @see setup for committing changes
  void remove_local(Uint lid, bool on_all_ranks=false);

  //@} END COMMPATTERN HANDLING

  /// @name ACCESSORS
  //@{

  /// accessor to check if setup is needed to call or not
  /// @return true or false, respectively
  bool isUpToDate() const { return m_isUpToDate; }

  /// accessor to check if compattern is frozen or not
  /// @return true or false, respectively
  bool isFreeze() const { return m_isFreeze; }

  /// accessor to global indexing
  /// @return const CommWrapper pointer to the data
  const Handle<CommWrapper> gid() const { return m_gid; }

  /// accessor to the m_isUpdatable vector
  /// @return vector of bools
  std::vector<bool>& isUpdatable() { return m_isUpdatable; }

  /// Return the rank associated with the given local ID
  int rank(const Uint lid) const { return m_ranks[lid]; }

  //@} END ACCESSORS

protected: // helper function

  /// function to synchronize this object
  /// useful for reusing in the different synchronize functions
  /// @param pobj reference to commwrapper object to synchronize to
  /// @param sndbuf vector for intermediate buffer for send
  /// @param rcvbuf vector for intermediate buffer for recieve
  void synchronize_this( const CommWrapper& pobj, std::vector<unsigned char>& sndbuf, std::vector<unsigned char>& rcvbuf );

private:

  /// @name PROPERTIES
  //@{

  /// flag telling if communication pattern is up-to-date (there are no items )
  bool m_isUpToDate;

  /// flag telling if pattern are set not to be allowed to change
  bool m_isFreeze;

  //@} END PROPERTIES

  /// @name BUFFERS HOLDING TEMPORARY DATA, TILL SETUP IS CALLED
  //@{

  /// temporary buffer of add
  /// id is global id
  temp_buffer_array m_add_buffer;

  /// temporary buffer of move
  /// id is local id
  temp_buffer_array m_mov_buffer;

  /// temporary buffer of remove
  /// id is local id
  temp_buffer_array m_rem_buffer;

  /// temporary buffer storing the available local ids, which are scattered inside the already existing data
  std::vector<Uint> m_free_lids;

  //@} END BUFFERS HOLDING TEMPORARY DATA

  /// explicit shared_ptr to the gid wrapper
  Handle<CommWrapper> m_gid;

  /// array holding the updatable info
  std::vector<bool> m_isUpdatable;

  /// this is the process-wise counter of sending communication pattern
  std::vector< CPint > m_sendCount;

  /// this is the map of sending communication pattern
  std::vector< CPint > m_sendMap;

  /// this is the process-wise counter of receiveing communication pattern
  std::vector< CPint > m_recvCount;

  /// this is the map of receiveing communication pattern
  std::vector< CPint > m_recvMap;

  /// Rank for all the gids in local index space
  std::vector<int> m_ranks;

}; // CommPattern

////////////////////////////////////////////////////////////////////////////////////////////

} // PE
} // common
} // cf3

#endif // cf3_common_PE_CommPattern_hpp
