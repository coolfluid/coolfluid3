// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file ComponentIterator.hpp
/// @brief Holds the ComponentIterator class

#ifndef cf3_common_ComponentIterator_hpp
#define cf3_common_ComponentIterator_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/iterator/iterator_facade.hpp>

#include <common/Handle.hpp>

namespace cf3 {
namespace common {

/// @brief %ComponentIterator class, can linearize a complete tree of components
///
/// The ComponentIterator class is the type for
/// - Component::iterator
/// - Component::const_iterator
///
/// - Using Component::begin() and Component::end() iterates on only 1 deeper level
/// - Using Component::recursive_begin() and Component::recursive_end() iterates
/// on all deeper levels recursively. Iterating will then linearize the tree.

template<class T>
class ComponentIterator :
    public boost::iterator_facade<ComponentIterator<T>,  // iterator
                                  T,                     // Value
                                  boost::bidirectional_traversal_tag, // search direction
                                  T&                     // return type of dereference
                                 >
{
  typedef boost::iterator_facade<ComponentIterator<T>,
                                 T,
                                 boost::random_access_traversal_tag,
                                 T&> BaseT;
public:

  typedef typename BaseT::difference_type difference_type;

  /// Construct an iterator over the given set of components.
  /// If endIterator is true, the iterator is intialized
  /// at the end of the range, otherwise at the beginning.
  explicit ComponentIterator(const std::vector<boost::shared_ptr<T> >& vec,
                             const Uint startPosition)
          : m_vec(vec), m_position(startPosition) {}

private:
  friend class boost::iterator_core_access;
  template <class> friend class ComponentIterator;

  template <typename T2>
  bool equal(ComponentIterator<T2> const& other) const { return (m_position == other.m_position); }

  void increment()
  {
    cf3_assert(m_position != m_vec.size());
    ++m_position;
  }

  void decrement()
  {
    cf3_assert(m_position != 0);
    --m_position;
  }

  void advance(const difference_type n) { m_position += n; }

  template <typename T2>
  difference_type distance_to(ComponentIterator<T2> const& other) const
  {
    return other.m_position - m_position;
  }

public:

  /// dereferencing
  T& dereference() const { return *m_vec[m_position]; }
  /// Get a handle to the referenced object
  Handle<T> get() const { return Handle<T>(m_vec[m_position]); }
  /// Compatibility with boost filtered_iterator interface,
  /// so base() can be used transparently on all ranges
  ComponentIterator<T>& base() { return *this; }
  /// Compatibility with boost filtered_iterator interface
  const ComponentIterator<T>& base() const { return *this; }

private:
  std::vector<boost::shared_ptr<T> > m_vec;
  Uint m_position;
};

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

#endif // cf3_common_ComponentIterator_hpp
