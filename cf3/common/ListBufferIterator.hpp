// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_ListBufferIterator_hpp
#define cf3_common_ListBufferIterator_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include "common/CF.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

template<class BufferT>
class ListBufferIterator
        : public boost::iterator_facade<ListBufferIterator<BufferT>,        // iterator
                                        typename BufferT::value_type,                // Value
                                        boost::bidirectional_traversal_tag, // search direction
																				typename BufferT::value_type&
																			 >
{
  typedef boost::iterator_facade<ListBufferIterator<BufferT>, BufferT, boost::random_access_traversal_tag, typename BufferT::value_type&> BaseT;
public:

  typedef typename BaseT::difference_type difference_type;

  /// Construct an iterator over the given set of components. If endIterator is true, the iterator is intialized
  /// at the end of the range, otherwise at the beginning.
  explicit ListBufferIterator(BufferT& buffer, const Uint startPosition)
          : m_buffer(buffer), m_position(startPosition)
  {
  }

private:
  friend class boost::iterator_core_access;
  template <class> friend class ListBufferIterator;

  template <typename BufferT2>
  bool equal(ListBufferIterator<BufferT2> const& other) const
  {
    return (m_position == other.m_position);
  }

  void increment()
  {
    // cf3_assert(m_position != m_buffer.total_allocated());
    ++m_position;
  }

  void decrement()
  {
    cf3_assert(m_position != 0);
    --m_position;
  }

  void advance(const difference_type n)
  {
    m_position += n;
  }

  template <typename BufferT2>
  difference_type distance_to(ListBufferIterator<BufferT2> const& other) const
  {
    return other.m_position - m_position;
  }

public:
  typename BufferT::value_type& dereference() const
  {
    return m_buffer.get_row(m_position);
  }
  
  /// Compatibility with boost filtered_iterator interface, so base() can be used transparently on all ranges
  ListBufferIterator<BufferT>& base()
  {
    return *this;
  }
  
  const ListBufferIterator<BufferT>& base() const
  {
    return *this;
  }

private:
  BufferT& m_buffer;
  Uint m_position;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_ListBufferIterator_hpp
