#ifndef CF_Common_ComponentIterator_hpp
#define CF_Common_ComponentIterator_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/iterator/iterator_facade.hpp>

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

template<class T>
class ComponentIterator
        : public boost::iterator_facade<ComponentIterator<T>,        // iterator
                                        T,                            // Value
                                        boost::bidirectional_traversal_tag, // search direction
                                        T&                            // return type of dereference is a reference
                                       >
{
  typedef boost::iterator_facade<ComponentIterator<T>, T, boost::random_access_traversal_tag, T&> base;
public:

  typedef typename base::difference_type difference_type;

  /// Construct an iterator over the given set of components. If endIterator is true, the iterator is intialized
  /// at the end of the range, otherwise at the beginning.
  explicit ComponentIterator(std::vector<boost::shared_ptr<T> > vec, const Uint startPosition)
          : m_vec(vec), m_position(startPosition)
  {
  }

private:
  friend class boost::iterator_core_access;
  template <class> friend class ComponentIterator;

  template <typename T2>
  bool equal(ComponentIterator<T2> const& other) const
  {
    return (m_position == other.m_position);
  }

  void increment()
  {
    cf_assert(m_position != m_vec.size());
    ++m_position;
  }

  void decrement()
  {
    cf_assert(m_position != 0);
    --m_position;
  }

  void advance(const difference_type n)
  {
    m_position += n;
  }

  template <typename T2>
  difference_type distance_to(ComponentIterator<T2> const& other) const
  {
    return other.m_position - m_position;
  }

public:
  T& dereference() const
  {
    return *m_vec[m_position];
  }

  /// Get a shared pointer to the referenced object
  boost::shared_ptr<T> get() const
  {
    return m_vec[m_position];
  }

private:
  std::vector<boost::shared_ptr<T> > m_vec;
  Uint m_position;
};

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ComponentIterator_hpp
