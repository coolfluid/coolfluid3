// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_PECommPattern_hpp
#define CF_Common_MPI_PECommPattern_hpp

////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "Common/Component.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/PEObjectWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/**
  @file PECommPattern.hpp
  @author Tamas Banyai
  @brief Parallel Communication Pattern.
  This class provides functionality to collect communication.
  For efficiency it works such a way that you submit your request via the constructor or the add/remove/move magic triangle and then call setup to modify the commpattern.
  The data needed to be kept synchronous can be registered via the insert function.
  The word node here means any kind of "point of storage", in this context it is not directly related with the computational mesh.
**/

// TODO!!!
/**
  @todo make possible to use compattern on non-registered data
  @todo when adding, how to give values to the newly createable elements?
  @todo add readonly properties (for example for coordinates, you want to keep it synchronous with commpattern but don't actually want to update it every time)
  @todo propagate CPint through mpiwrapper
  @todo gid registration: must be more straightforward to check if its really a CFuint single stride data
  @todo introduce allocate_component
**/

class Common_API PECommPattern: public Component {

public:

  /// @name TYPEDEFS
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<PECommPattern> Ptr;
  /// const pointer to this type
  typedef boost::shared_ptr<PECommPattern const> ConstPtr;

  /// type of integer to use internally in commpattern, to avoid mess of changing type when mpi allows unsigned ints
  typedef int CPint;
  /// typedef for the temporary buffer
  class temp_buffer_item{
    public:
      temp_buffer_item(Uint _gid, Uint _rank, bool _option) { gid=(CPint)_gid; rank=(CPint)_rank; option=_option; }
      CPint gid;
      CPint rank;
      bool option;
  };
  /// typedef for the temporary buffer array
  typedef std::vector<temp_buffer_item> temp_buffer_array;

  /// typedef

  //@} TYPEDEFS

public:

  /// @name CONSTRUCTORS, DESTRUCTORS & OTHER BELONGINGS
  //@{

  /// constructor
  /// @param name under this name will the component be registered
  PECommPattern(const std::string& name);

  /// destructor
  ~PECommPattern();

  /// Get the class name
  static std::string type_name () { return "PECommPattern"; }

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
    typename PEObjectWrapperPtr<T>::Ptr ow = create_component< PEObjectWrapperPtr<T> >(name);
    ow->setup(data,stride,needs_update);
  }

  /// register data coming from pointer to naked pointer
  /// @param name the component will appear under this name
  /// @param data pointer to data
  /// @param size length of the data
  /// @param stride number of array element grouping
  template<typename T> void insert(const std::string& name, T** data, const int size, const unsigned int stride=1, const bool needs_update=true)
  {
    typename PEObjectWrapperPtr<T>::Ptr ow = create_component< PEObjectWrapperPtr<T> >(name);
    ow->setup(data,stride,needs_update);
  }

  /// register data coming from std::vector by reference
  /// @param name the component will appear under this name
  /// @param pointer to std::vector of data
  /// @param stride number of array element grouping
  template<typename T> void insert(const std::string& name, std::vector<T>& data, const unsigned int stride=1, const bool needs_update=true)
  {
    typename PEObjectWrapperVector<T>::Ptr ow = create_component< PEObjectWrapperVector<T> >(name);
    ow->setup(data,stride,needs_update);
  }

  /// register data coming from pointer to std::vector
  /// @param name the component will appear under this name
  /// @param pointer to std::vector of data
  /// @param stride number of array element grouping
  template<typename T> void insert(const std::string& name, std::vector<T>* data, const unsigned int stride=1, const bool needs_update=true)
  {
    typename PEObjectWrapperVector<T>::Ptr ow = create_component< PEObjectWrapperVector<T> >(name);
    ow->setup(data,stride,needs_update);
  }

  /// register data coming from std::vector wrapped into weak_ptr (also works with shared_ptr)
  /// @param name the component will appear under this name
  /// @param std::vector of data
  /// @param stride number of array element grouping
  template<typename T> void insert(const std::string& name, boost::weak_ptr< std::vector<T> > data, const unsigned int stride=1, const bool needs_update=true)
  {
    typename PEObjectWrapperVectorWeakPtr<T>::Ptr ow = create_component< PEObjectWrapperVectorWeakPtr<T> >(name);
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
  /// @param gid PEObjectWrapper to a Uint tpye of data array
  /// @param rank vector of ranks where given global ids are updatable to add
  void setup(PEObjectWrapper::Ptr gid, std::vector<Uint>& rank);

  /// build and/or modify communication pattern - only incorporate actual buffers
  /// this function sets actually up the communication pattern
  /// beware: interprocess communication heavy
  void setup();

  /// synchronize items
  void update();

  /// add element to the commpattern
  /// when all changes done, all needs to be committed by calling setup
  /// if global id is not on current rank, then a ghost is automatically created on current rank
  /// @param gid global id
  /// @param rank rank where given global node is to be updatable
  /// @see setup for committing changes
  void add(Uint gid, Uint rank);

  /// move element along partitions
  /// when all changes done, all needs to be committed by calling setup
  /// @param gid global id
  /// @param rank rank where the node is expected to be updatable
  /// @param keep_as_ghost decision if node to be kept on from_rank as ghost
  /// @see setup for committing changes
  void move(Uint gid, Uint rank, bool keep_as_ghost=true);

  /// delete element from the commpattern
  /// when all changes done, all needs to be committed by calling setup
  /// @param gid global id of the guy to be excluded from commpattern
  /// @param rank if on_all_ranks is true, its a complete removal. If its false then depends if node is ghost or updatable on rank and deletes only on this rank or all ranks respectively
  /// @param on_all_ranks force delete on all processes if true
  /// @see setup for committing changes
  void remove(Uint gid, Uint rank, bool on_all_ranks=false);

  //@} END COMMPATTERN HANDLING

  /// @name ACCESSORS
  //@{

  /// accessor to check if setup is needed to call or not
  /// @return true or false, respectively
  bool isUpToDate() const { return m_isUpToDate; }

  /// accessor to check if setup is needed to call or not
  /// @return true or false, respectively
  bool isFreeze() const { return m_isFreeze; }

  //@} END ACCESSORS

private:

  /// @name PROPERTIES
  //@{

  /// flag telling if communication pattern is up-to-date (there are no items )
  bool m_isUpToDate;

  /// flag telling if communication pattern is up-to-date (there are no items )
  bool m_isFreeze;

  //@} END PROPERTIES

  /// @name BUFFERS HOLDING TEMPORARY DATA, TILL SETUP IS CALLED
  //@{

  /// temporary buffer of add
  temp_buffer_array m_add_buffer;

  /// temporary buffer of move
  temp_buffer_array m_mov_buffer;

  /// temporary buffer of remove
  temp_buffer_array m_rem_buffer;

  //@} END BUFFERS HOLDING TEMPORARY DATA

  /// explicit shared_ptr to the gid wrapper
  PEObjectWrapper::Ptr m_gid;

  /// array holding the updatable info
  std::vector<bool> m_isUpdatable;

/*
  /// Storing updatable information.
  /// Note that this is not containing the full length of the array, only the part is involved in the communication.
  std::vector<bool> m_updatable;

  /// this is the global id
  /// Note that this is not containing the full length of the array, only the part is involved in the communication.
  std::vector< Uint > gid;

  /// this is the process-wise counter of sending communication pattern
  std::vector< int > m_sendCount;

  /// this is the map of sending communication pattern
  std::vector< int > m_sendMap;

  /// this is the process-wise counter of receiveing communication pattern
  std::vector< int > m_receiveCount;

  /// this is the map of receiveing communication pattern
  std::vector< int > m_receiveMap;

  /// flag telling if communication pattern is up-to-date (there are no items )
  bool m_isCommPatternSetupNeeded;
*/

}; // PECommPattern

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_PECommPattern_hpp
