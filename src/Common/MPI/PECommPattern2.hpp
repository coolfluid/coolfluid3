// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_PECommPattern2_hpp
#define CF_Common_MPI_PECommPattern2_hpp

////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "Common/Component.hpp"
#include "Common/MPI/PEObjectWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/**
  @file PECommPattern2.hpp
  @author Tamas Banyai
  Parallel Communication Pattern.
  This class provides functionality to collect communication.
  For efficiency it works such a way that you submit your request via the constructor or the add/remove/move magic triangle and then call setup to modify the commpattern.
  The data needed to be kept synchronous can be registered via the insert function
**/

// TODO:
// 1.: make possible to use compattern on non-registered data
// 2.: when adding, how to give values to the newly createable elements?

class Common_API PECommPattern2: public Component {

public:

  /// @name TYPEDEFS
  //@{

  /// provider
  typedef Common::ConcreteProvider < PECommPattern2,1 > PROVIDER;
  /// pointer to this type
  typedef boost::shared_ptr<PECommPattern2> Ptr;
    /// const pointer to this type
  typedef boost::shared_ptr<PECommPattern2 const> ConstPtr;

  //@} TYPEDEFS

public:

  /// @name CONSTRUCTORS, DESTRUCTORS & OTHER BELONGINGS
  //@{

  /// constructor
  /// @param name under this name will the component be registered
  PECommPattern2(const CName& name);

  /// constructor with settting up communication pattern
  /// don't forget to commit changes by using setup
  /// @param name under this name will the component be registered
  /// @param gid vector of global ids
  /// @param rank vector of ranks where given global ids are updatable
  /// @see setup for committing changes
  PECommPattern2(const CName& name, std::vector<Uint> gid, std::vector<Uint> rank);

  /// destructor
  ~PECommPattern2();

  /// Get the class name
  static std::string type_name () { return "PECommPattern2"; }

  //@} END CONSTRUCTORS/DESTRUCTORS

  /// @name DATA REGISTRATION
  //@{

  /// register data coming from naked pointer by reference
  /// @param name the component will appear under this name
  /// @param data pointer to data
  /// @param size length of the data
  /// @param stride number of array element grouping
  template<typename T> void insert(const CName& name, T*& data, const int size, const unsigned int stride=1)
  {
    boost::shared_ptr< PEObjectWrapper > ow( new PEObjectWrapperPtr<T>(name,data,size,stride) );
    add_component ( ow );
  }

  /// register data coming from pointer to naked pointer
  /// @param name the component will appear under this name
  /// @param data pointer to data
  /// @param size length of the data
  /// @param stride number of array element grouping
  template<typename T> void insert(const CName& name, T** data, const int size, const unsigned int stride=1)
  {
    boost::shared_ptr< PEObjectWrapper > ow( new PEObjectWrapperPtr<T>(name,data,size,stride) );
    add_component ( ow );
  }

  /// register data coming from std::vector by reference
  /// @param name the component will appear under this name
  /// @param pointer to std::vector of data
  /// @param stride number of array element grouping
  template<typename T> void insert(const CName& name, std::vector<T>& data, const unsigned int stride=1)
  {
    boost::shared_ptr< PEObjectWrapper > ow( new PEObjectWrapperVector<T>(name,data,stride) );
    add_component ( ow );
  }

  /// register data coming from pointer to std::vector
  /// @param name the component will appear under this name
  /// @param pointer to std::vector of data
  /// @param stride number of array element grouping
  template<typename T> void insert(const CName& name, std::vector<T>* data, const unsigned int stride=1)
  {
    boost::shared_ptr< PEObjectWrapper > ow( new PEObjectWrapperVector<T>(name,data,stride) );
    add_component ( ow );
  }

  /// register data coming from std::vector wrapped into weak_ptr (also works with shared_ptr)
  /// @param name the component will appear under this name
  /// @param std::vector of data
  /// @param stride number of array element grouping
  template<typename T> void insert(const CName& name, boost::weak_ptr< std::vector<T> > data, const unsigned int stride=1)
  {
    boost::shared_ptr< PEObjectWrapper > ow( new PEObjectWrapperVectorWeakPtr<T>(name,data,stride) );
    add_component ( ow );
  }

  /// removes data by name
  void clear( const CName& name)
  {
    remove_component(name);
  }

  //@} END DATA REGISTRATION

/*

  /// build and/or modify communication pattern
  /// this function sets actually up the communication pattern
  /// beware: interprocess communication heavy
  /// @param gid vector of global ids
  /// @param rank vector of ranks where given global ids are updatable
  void setup(std::vector<Uint> gid, std::vector<Uint> rank);

  /// build and/or modify communication pattern
  /// this function sets actually up the communication pattern
  /// beware: interprocess communication heavy
  void setup();

  /// add element to the commpattern
  /// when all changes done, all needs to be committed by calling setup
  /// @param gid global id
  /// @param rank rank where given global id is updatable
  /// @see setup for committing changes
  void add(Uint gid, Uint rank);

  /// delete element from the commpattern
  /// when all changes done, all needs to be committed by calling setup
  /// @param gid global id
  /// @param rank rank where given global id is updatable
  /// @see setup for committing changes
  void remove(Uint gid, Uint rank);

  /// move element along partitions
  /// when all changes done, all needs to be committed by calling setup
  /// @param gid global id
  /// @param rank rank where given global id is updatable
  /// @see setup for committing changes
  void move(Uint gid, Uint rank);

  /// registers data
  template <typename T> insert( );

  /// releases data
  template <typename T> release(T*);

  /// synchronize items
  void sync();

  /// clears the communication pattern and releases every data associated to commpattern
  void clear();

  /// accessor to the bool telling if there are modifications pending for the commpattern, which needs to be committed by calling setup
  const bool isCommPatternSetupNeeded() { return (const bool)m_isCommPatternSetupNeeded; }

*/

  /// accessor to check if setup is needed to call or not
  /// @return true or false, respectively
  const bool isUpToDate() { return m_isUpToDate; }

private:

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self );

private:

  /// flag telling if communication pattern is up-to-date (there are no items )
  bool m_isUpToDate;

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

};


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_PECommPattern2_hpp
