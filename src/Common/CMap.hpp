// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CMap_hpp
#define CF_Common_CMap_hpp

////////////////////////////////////////////////////////////////////////////////

#include <map>

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This component class represents a "cheap" (from the point of view of the 
/// memory requirements) map with SINGLE key  that gives the same
/// performance in searching of a std::map (the number
/// of comparisons is <= 2*log(size)+1. Since the high memory storage is
/// an issue with the standard <map>, this class offer a valid efficient
/// alternative to it.
/// Consider that once that you allocate a map, you can't get rid of the
/// allocated memory till the end of your run! This doesn't apply to the current
/// CMap that is just an Adapter of std::vector<std::pair<KEY,DATA> >.
/// THIS IS REALLY AN IMPORTANT ADVANTAGE OF THIS MAP: since it is basically
/// a std::vector, YOU CAN RELEASE THE MEMORY when you go out of scope or you explicitly
/// delete a pointer to this object.
/// The class is parameterized with the types of the key and the type of the value
/// @pre this map can handle KEYs for which the operators "<" and ">" are available
/// @pre before using find() the CMap has to be sorted. This will happen automatically
/// @pre the situation in which this map must be used is a situation in which you first
///      insert ALL the pairs, then you sort (sort_keys() is a demanding operation
///      that must be applied only once!!) and then you start searching with find().
/// Consider using 2 seperate vectors (one for keys and one for elements)
/// It will be easier on the memory allocator, and it will provide for faster
/// access (multiple keys will be loaded in one cacheline when searching)
/// @author Andrea Lani
/// @author Tiago Quintino  
/// @author Willem Deconinck
template <typename KEY, typename DATA>  
class Common_API CMap : public Component {

public: // typedefs

  /// @typedef Associative Container	 The map's key type, Key.
  typedef KEY key_type;
  /// @typedef Pair Associative Container	 The type of object associated with the keys.
  typedef DATA data_type;
  /// @typedef The type of object, pair<const key_type, data_type>, stored in the map.
  typedef std::pair<key_type, data_type> value_type;
  
  /// @typedef iterator definition for use in stl algorithms
  typedef typename std::vector<value_type>::iterator         iterator;
  /// @typedef const_iterator definition for use in stl algorithms
  typedef typename std::vector<value_type>::const_iterator   const_iterator;

  /// shared pointer to this type
  typedef boost::shared_ptr<CMap> Ptr;
  typedef boost::shared_ptr<CMap const> ConstPtr;

public: // functions

  /// Contructor
  /// @param[in] name of the component
  CMap ( const CName& name ) : Component(name)
  {
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~CMap() {}

  /// Get the class name
  static std::string type_name () { return "CMap"; }

  /// Configuration Options
  static void define_config_properties ( Common::PropertyList& options ) {}
  
  /// Reserve memory
  /// @param[in] size of the map to be set before starting inserting pairs in the  map
  /// @post  the memory corresponding to the given size will be reserved for
  ///        the future insertions.
  void reserve (size_t max_size);

  /// Copy a std::map into the CMap
  /// @param[in] map The map to copy
  void copy_std_map (std::map<key_type,data_type>& map);
  
  /// Insert pair
  /// @param[in] key   new key to be inserted
  /// @param[in] data  new data to be inserted, corresponding to the given key
  /// @post a new std::pair<KEY, DATA> will be created and pushed back in the map
  /// @post the capacity of the map will increase only if the memory hasn't been
  ///       reserved properly at start up.
  Uint insert_blindly(const key_type& key, const data_type& data);

  /// Insert pair the same way a std::map would.
  /// @note WARNING: this will cause a costly std::sort on the internal structure
  ///                if sorting did not happen yet.
  /// @param[in] v   std::pair<KEY,DATA> type to 
  /// @returns a pair, with its member pair::first set to an 
  ///          iterator pointing to either the newly inserted element or to the element
  ///          that already had its same value in the map. The pair::second element in 
  ///          the pair is set to true if a new element was inserted or false if an element
  ///          with the same value existed.
  /// @post a new std::pair<KEY, DATA> will be created and pushed back in the map
  /// @post the capacity of the map will increase only if the memory hasn't been
  ///       reserved properly at start up.
  std::pair<iterator,bool> insert(const value_type& v);

  /// Find the iterator matching with the given KEY
  /// @param[in] key  key to be looked-up
  /// @pre Before using find() the CMap has to be sorted with sort_keys()
  ///      This happens automatically
  /// @post If the key is not in the map, the end() iterator is returned.
  /// @return the iterator with key and value, the end() iterator is returned
  ///         if no match is found
  /// @exception NoSuchValueException is thrown if key not in map
  iterator find(const key_type& key);
  
  /// Find the iterator matching with the given KEY
  /// @param[in] key  key to be looked-up
  /// @pre Before using find() the CMap has to be sorted with sort_keys()
  ///      This cannot happen automatically in this const version
  /// @post If the key is not in the map, the end() iterator is returned.
  /// @return the iterator with key and value, the end() iterator is returned
  ///         if no match is found
  /// @exception IllegalCall is thrown if the CMap is not sorted beforehand
  const_iterator find(const key_type& key) const;
  
  /// Erase the given iterator from the map
  /// @param[in] itr The iterator to delete
  /// @pre the map must be sorted
  /// @exception IllegalCall is thrown when map is not sorted
  void erase (iterator itr)
  {
    if (!m_sorted)
      throw IllegalCall(FromHere(), "Map internal structure must be sorted");
    
    m_vectorMap.erase(itr);
    m_sorted = false;
  }
  
  /// Erase the entry wit given key from the map
  /// @note WARNING: This operation will call the costly sort_keys() function
  ///                if it the internal structure is not sorted.
  /// @param[in] itr The iterator to delete
  /// @returns true if element is erased, false if no element was erased
  bool erase (const key_type& key)
  {
    iterator itr = find(key);
    if (itr == end())
      return false;
    m_vectorMap.erase(itr);
    m_sorted = false;
    return true;
  }
  
  /// Check if the given KEY is existing in the CMap
  /// @param[in] key  key to be looked-up
  /// @pre Before using exists() the CFMap has to be sorted with sort_keys().
  ///      This happens automatically
  /// @return flag to know if key exists
  bool exists(const key_type& key);
  
  /// Check if the given KEY is existing in the CMap
  /// @param[in] key  key to be looked-up
  /// @pre before using exists() the CFMap has to be sorted with sort_keys()
  /// @return flag to know if key exists
  /// @exception IllecalCall is thrown if the CMap is not sorted beforehand
  bool exists(const key_type& key) const;

  /// Clear the content of the map
  void clear();

  /// Get the number of pairs already inserted
  size_t size() const;

  /// Overloading of the operator"[]" for assignment AND insertion
  /// @note WARNING: This procedure will call the costly sort_keys() if the map is not sorted
  /// @param[in] key The key to look for. If the key is not found,
  ///               it is inserted using insert_blindly().
  /// @return modifiable data. In case the key did not exist, this will assign the newly created data.
  data_type& operator[] (const key_type& key);

  /// Overloading of the operator"[]" for assignment AND insertion
  /// @note WARNING: This procedure will call the costly sort_keys() if the map is not sorted
  /// @param[in] key The key to look for. If the key is not found,
  ///               it is inserted using insert_blindly().
  /// @return non-modifiable data for the given key
  /// @exception IllegalCall is thrown when the map is unsorted.
  /// @exception ValueNotFound is thrown when the key does not exist in the map, as it cannot be inserted.
  const data_type& operator[] (const key_type& key) const;

  /// Sort all the pairs in the map by key
  /// @exception ValueExists is thrown when duplicate keys are discovered.
  void sort_keys();
  
  /// @return the iterator pointing at the first element
  iterator begin();
  
  /// @return the const_iterator pointing at the first element
  const_iterator begin() const;

  /// @return the end iterator
  iterator end();
  
  /// @return the end const_iterator
  const_iterator end() const;
  
private: // nested classes

  /// This class represents a functor object that is passed as an argument
  /// in the std::sort algo to compare two given pairs.
  /// @post the pairs are sorted in such a way that their KEYs are
  ///       listed in increasing order.
  /// @author Andrea Lani
  class LessThan {
  public:

    /// Overloading of the operator() that makes this class acting as a functor
    /// @param p1  pair to be used as first term of comparison
    /// @param p2  pair to be used as second term of comparison
    /// @return true  if(p1.first < p2.first)
    /// @return false if(p1.first >= p2.first)
    /// @post the pairs will be ordered according to the increasing order of
    ///       their keys
    /// @post sortKeys() uses these function to order the inserted pairs
    bool operator() (const std::pair<KEY,DATA>& p1,
                       const std::pair<KEY,DATA>& p2) const
    {
      return (p1.first < p2.first) ? true : false;
    }
  };
  
  /// This class represents a functor object that is used to compare a
  /// given key with the key in a std::pair. This functor is passed as
  /// an argument in the std::equal_range function in order to find all
  /// the pairs containing the specified key.
  /// @author Andrea Lani
  class Compare {
  public:

    /// Overloading of the operator() that makes this class acting as a functor
    /// @param p1   pair whose key is used as first term of comparison
    /// @param key  given key used as second term of comparison
    /// @return true  if(p1.first < key)
    /// @return false if(p1.first >= key)
    /// @post this is the first test to see if p1.first is == key during the
    ///       search with find()
    bool operator() (const std::pair<KEY,DATA>& p1, const KEY& key) const
    {
      return (p1.first < key) ? true : false;
    }

    /// Overloading of the operator() that makes this class acting as a functor
    /// @param key  given key used as first term of comparison
    /// @param p1   pair whose key is used as second term of comparison
    /// @return true  if(p1.first > key)
    /// @return false if(p1.first <= key)
    /// @post this is the second test to see if p1.first is == key during the
    ///       search with find()
    bool operator() (const KEY& key, const std::pair<KEY,DATA>& p1) const
    {
      return (p1.first > key) ? true : false;
    }

  }; // class Compare
  
private: // helper functions

  /// Helper function to discover if two map entries have the same key
  /// @param[in] val1 One of the two entries to compare the key of
  /// @param[in] val2 One of the two entries to compare the key of
  /// @returns true if duplicate keys are found
  static bool unique_key(const value_type& val1, const value_type& val2);

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: //data

  /// Keeps track of the validity of the map
  /// if a key has been inserted and the map
  /// has not yet been sorted, then it this will be false
  bool m_sorted;

  /// storage of the inserted data
  std::vector<value_type>  m_vectorMap;

};

////////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline void CMap<KEY,DATA>::reserve (size_t max_size)
{
  m_vectorMap.reserve(max_size);
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
void CMap<KEY,DATA>::copy_std_map (std::map<key_type,data_type>& map)
{
  reserve(map.size());
  
  typename std::map<key_type,data_type>::iterator itr = map.begin();
  typename std::map<key_type,data_type>::iterator map_end = map.end();
  for(; itr != map_end; ++itr)
    insert_blindly(itr->first,itr->second);
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline Uint CMap<KEY,DATA>::insert_blindly(const key_type& key, const data_type& data)
{
  m_sorted = false;
  m_vectorMap.push_back(value_type(key,data));
  return m_vectorMap.size()-1;
} 

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline std::pair<typename CMap<KEY,DATA>::iterator,bool> CMap<KEY,DATA>::insert(const value_type& v)
{
  m_sorted = false;
  iterator itr = find(v.first);
  if (itr == end())
  {
    insert_blindly(v.first,v.second);
    return std::make_pair(find(v.first),true);
  }
  else
  {
    return std::make_pair(itr,false);
  }
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename CMap<KEY,DATA>::iterator CMap<KEY,DATA>::find(const key_type& key)
{
  if (m_vectorMap.empty())
    return end();

  if (!m_sorted)
    sort_keys();

  iterator itr = std::lower_bound(begin(),end(),key,Compare());

  if (itr != end())       
    if (itr->first != key)
      return end();

  return itr;
}
  
//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename CMap<KEY,DATA>::const_iterator CMap<KEY,DATA>::find(const key_type& key) const
{
  if(m_vectorMap.empty())
    return end();
   
  if(!m_sorted)
    throw IllegalCall(FromHere(), "Trying to sort CMap is not allowed in find() \"const\". use sort_keys() apriori");
   
  return std::lower_bound(begin(),end(),key,Compare());
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA> 
inline bool CMap<KEY,DATA>::exists(const key_type& key)
{
  return (find(key) != end());
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
bool CMap<KEY,DATA>::exists(const key_type& key) const
{
  return (find(key) != end());
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline void CMap<KEY,DATA>::clear()
{
  std::vector<value_type>().swap(m_vectorMap);
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
size_t CMap<KEY,DATA>::size() const
{
  return m_vectorMap.size();
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename CMap<KEY,DATA>::data_type& CMap<KEY,DATA>::operator[] (const key_type& key)
{
  iterator itr = find(key);
  if (itr != end())
    return itr->second;
  else
  {
    return m_vectorMap[insert_blindly(key,data_type())].second;
  }
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline const typename CMap<KEY,DATA>::data_type& CMap<KEY,DATA>::operator[] (const key_type& key) const
{
  const_iterator itr = find(key);
  if (itr != end())
    return itr->second;
  else
    throw ValueNotFound(FromHere(), "The key is not found in the CMap, and can not be inserted in const version.");
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
void CMap<KEY,DATA>::sort_keys()
{
  if (!m_sorted)
  {
    std::sort(m_vectorMap.begin(),m_vectorMap.end(), LessThan());
    m_sorted = true;

    iterator itr = std::unique (m_vectorMap.begin(), m_vectorMap.end(), unique_key );    
    if ( itr - begin() != size() )
      throw ValueExists (FromHere(), "multiple keys in the map are detected. Not allowed in this map");      
  }
}
  
//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename CMap<KEY,DATA>::iterator CMap<KEY,DATA>::begin()
{
  return m_vectorMap.begin();
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename CMap<KEY,DATA>::const_iterator CMap<KEY,DATA>::begin() const
{
  return m_vectorMap.begin();
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename CMap<KEY,DATA>::iterator CMap<KEY,DATA>::end()
{
  return m_vectorMap.end();
}
  
//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename CMap<KEY,DATA>::const_iterator CMap<KEY,DATA>::end() const
{
  return m_vectorMap.end();
}
  
//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline bool CMap<KEY,DATA>::unique_key(const value_type& val1, const value_type& val2)
{
  return (val1.first==val2.first);
}

//////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CMap_hpp
