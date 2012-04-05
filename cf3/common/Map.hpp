// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Map_hpp
#define cf3_common_Map_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// This component class represents a "cheap" (from the point of view of the 
/// memory requirements) map with SINGLE key  that gives the same
/// performance in searching of a std::map (the number
/// of comparisons is <= 2*log(size)+1. Since the high memory storage is
/// an issue with the standard std::map, this class offer a valid efficient
/// alternative to it.
/// Consider that once that you allocate a map, you can't get rid of the
/// allocated memory till the end of your run! This doesn't apply to the current
/// Map that is just an Adapter of std::vector<std::pair<KEY,DATA> >.
/// THIS IS REALLY AN IMPORTANT ADVANTAGE OF THIS MAP: since it is basically
/// a std::vector, YOU CAN RELEASE THE MEMORY when you go out of scope or you explicitly
/// delete a pointer to this object.
/// The class is parameterized with the types of the key and the type of the value
/// @pre this map can handle KEYs for which the operators "<" and ">" are available
/// @pre before using find() the Map has to be sorted. This will happen automatically
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
class Map : public Component {

public: // typedefs

  /// @brief Associative Container -- The map's key type, Key.
  typedef KEY key_type;
  /// @brief Pair Associative Container -- The type of object associated with the keys.
  typedef DATA data_type;
  /// @brief The type of object, pair<const key_type, data_type>, stored in the map.
  typedef std::pair<key_type, data_type> value_type;
  
  /// @brief iterator definition for use in stl algorithms
  typedef typename std::vector<value_type>::iterator         iterator;
  /// @brief const_iterator definition for use in stl algorithms
  typedef typename std::vector<value_type>::const_iterator   const_iterator;

public: // functions

  /// Contructor
  /// @param[in] name of the component
  Map ( const std::string& name ) : Component(name)
  {
		regist_typeinfo(this);
  }

  /// Virtual destructor
  virtual ~Map() {}

  /// Get the class name
  static std::string type_name () { return "Map<"+common::class_name<KEY>()+","+common::class_name<DATA>()+">"; }

  /// @brief Reserve memory
  /// @param[in] max_size of the map to be set before starting inserting pairs in the  map
  /// @post  the memory corresponding to the given size will be reserved for
  ///        the future insertions
  void reserve (size_t max_size);

  /// @brief Copy a std::map into the Map
  /// @param[in] map The map to copy
  void copy_std_map (std::map<key_type,data_type>& map);
  
  /// @brief Insert pair without any checks if it is already present
  ///
  /// This is the recommended way to add entries to the map, if you are
  /// sure that no entries will be duplicated
  /// @param[in] key   new key to be inserted
  /// @param[in] data  new data to be inserted, corresponding to the given key
  /// @post a new std::pair<KEY, DATA> will be created and pushed back in the map
  /// @post the capacity of the map will increase only if the memory hasn't been
  ///       reserved properly at start up.
  Uint push_back(const key_type& key, const data_type& data);

  /// @brief Insert pair the same way a std::map would.
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

  /// @brief Find the iterator matching with the given KEY
  /// @param[in] key  key to be looked-up
  /// @pre Before using find() the Map has to be sorted with sort_keys()
  ///      This happens automatically
  /// @post If the key is not in the map, the end() iterator is returned.
  /// @return the iterator with key and value, the end() iterator is returned
  ///         if no match is found
  iterator find(const key_type& key);
  
  /// @brief Find the iterator matching with the given KEY
  /// @param[in] key  key to be looked-up
  /// @pre Before using find() the Map has to be sorted with sort_keys()
  ///      This cannot happen automatically in this const version
  /// @post If the key is not in the map, the end() iterator is returned.
  /// @return the iterator with key and value, the end() iterator is returned
  ///         if no match is found
  const_iterator find(const key_type& key) const;
  
  /// @brief Erase the given iterator from the map
  /// @param[in] itr The iterator to delete
  /// @pre the map must be sorted
  void erase (iterator itr)
  {
    cf3_assert_desc ( "Map internal structure must be sorted" , m_sorted);
    m_vectorMap.erase(itr);
    m_sorted = false;
  }
  
  /// @brief Erase the entry wit given key from the map
  /// @note WARNING: This operation will call the costly sort_keys() function
  ///                if it the internal structure is not sorted.
  /// @param[in] key The iterator to delete
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
  
  /// @brief Check if the given KEY is existing in the Map
  /// @param[in] key  key to be looked-up
  /// @pre Before using exists() the CFMap has to be sorted with sort_keys().
  ///      This happens automatically
  /// @return flag to know if key exists
  bool exists(const key_type& key);
  
  /// @brief Check if the given KEY is existing in the Map
  /// @param[in] key  key to be looked-up
  /// @pre before using exists() the CFMap has to be sorted with sort_keys()
  /// @return flag to know if key exists
  bool exists(const key_type& key) const;

  /// @brief Clear the content of the map
  void clear();

  /// @brief Get the number of pairs already inserted
  size_t size() const;

  /// @brief Overloading of the operator"[]" for assignment AND insertion
  /// @note WARNING: This procedure will call the costly sort_keys() if the map is not sorted
  /// @param[in] key The key to look for. If the key is not found,
  ///               it is inserted using push_back().
  /// @return modifiable data. In case the key did not exist, this will assign the newly created data.
  data_type& operator[] (const key_type& key);

  /// @brief Overloading of the operator"[]" for assignment AND insertion
  /// @note WARNING: This procedure will call the costly sort_keys() if the map is not sorted
  /// @param[in] key The key to look for. If the key is not found,
  ///               it is inserted using push_back().
  /// @return non-modifiable data for the given key
  const data_type& operator[] (const key_type& key) const;

  /// @brief Sort all the pairs in the map by key
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
inline void Map<KEY,DATA>::reserve (size_t max_size)
{
  m_vectorMap.reserve(max_size);
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
void Map<KEY,DATA>::copy_std_map (std::map<key_type,data_type>& map)
{
  reserve(map.size());
  
  typename std::map<key_type,data_type>::iterator itr = map.begin();
  typename std::map<key_type,data_type>::iterator map_end = map.end();
  for(; itr != map_end; ++itr)
    push_back(itr->first,itr->second);
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline Uint Map<KEY,DATA>::push_back(const key_type& key, const data_type& data)
{
  m_sorted = false;
  m_vectorMap.push_back(value_type(key,data));
  return m_vectorMap.size()-1;
} 

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline std::pair<typename Map<KEY,DATA>::iterator,bool> Map<KEY,DATA>::insert(const value_type& v)
{
  m_sorted = false;
  iterator itr = find(v.first);
  if (itr == end())
  {
    push_back(v.first,v.second);
    return std::make_pair(find(v.first),true);
  }
  else
  {
    return std::make_pair(itr,false);
  }
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename Map<KEY,DATA>::iterator Map<KEY,DATA>::find(const key_type& key)
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
inline typename Map<KEY,DATA>::const_iterator Map<KEY,DATA>::find(const key_type& key) const
{
  if(m_vectorMap.empty())
    return end();
   
  cf3_assert_desc ("Trying to sort Map is not allowed in find() \"const\". use sort_keys() apriori", m_sorted );
   
  const_iterator itr = std::lower_bound(begin(),end(),key,Compare());

  if (itr != end())       
    if (itr->first != key)
      return end();

  return itr;
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA> 
inline bool Map<KEY,DATA>::exists(const key_type& key)
{
  return (find(key) != end());
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
bool Map<KEY,DATA>::exists(const key_type& key) const
{
  return (find(key) != end());
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline void Map<KEY,DATA>::clear()
{
  std::vector<value_type>().swap(m_vectorMap);
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
size_t Map<KEY,DATA>::size() const
{
  return m_vectorMap.size();
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename Map<KEY,DATA>::data_type& Map<KEY,DATA>::operator[] (const key_type& key)
{
  iterator itr = find(key);
  if (itr != end())
    return itr->second;
  else
  {
    return m_vectorMap[push_back(key,data_type())].second;
  }
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline const typename Map<KEY,DATA>::data_type& Map<KEY,DATA>::operator[] (const key_type& key) const
{
  const_iterator itr = find(key);
  cf3_assert_desc( "The key is not found in the Map, and can not be inserted in const version." , itr != end())
  return itr->second;
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
void Map<KEY,DATA>::sort_keys()
{
  if (!m_sorted)
  {
    std::sort(begin(), end(), LessThan());
    m_sorted = true;

    cf3_assert_desc ("multiple keys in the map are detected. Not allowed in this map" , 
      std::unique (begin(), end(), unique_key ) - begin() == (int) size() );  
  }
}
  
//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename Map<KEY,DATA>::iterator Map<KEY,DATA>::begin()
{
  return m_vectorMap.begin();
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename Map<KEY,DATA>::const_iterator Map<KEY,DATA>::begin() const
{
  return m_vectorMap.begin();
}

//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename Map<KEY,DATA>::iterator Map<KEY,DATA>::end()
{
  return m_vectorMap.end();
}
  
//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline typename Map<KEY,DATA>::const_iterator Map<KEY,DATA>::end() const
{
  return m_vectorMap.end();
}
  
//////////////////////////////////////////////////////////////////////////////

template <typename KEY, typename DATA>
inline bool Map<KEY,DATA>::unique_key(const value_type& val1, const value_type& val2)
{
  return (val1.first==val2.first);
}

//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Map_hpp
