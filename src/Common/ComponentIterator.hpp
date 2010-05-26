#ifndef CF_Common_ComponentIterator_hpp
#define CF_Common_ComponentIterator_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/iterator/iterator_facade.hpp>

namespace CF {
namespace Common {

  class Component;

////////////////////////////////////////////////////////////////////////////////

template<class T>
class Component_iterator
        : public boost::iterator_facade<Component_iterator<T>,        // iterator
                                        T,                            // Value
                                        boost::forward_traversal_tag, // search direction
                                        boost::shared_ptr<T>          // return type of dereference (NOTE IT IS A POINTER)
                                       >
{
public:

  explicit Component_iterator(const boost::shared_ptr<T>& parent)
    : m_parent(parent), m_component(parent) , m_counter(0)
  {
    m_vec.push_back(m_parent);
  }

  explicit Component_iterator(std::vector<boost::shared_ptr<T> > vec, const boost::shared_ptr<Component>& parent)
          : m_parent(parent), m_component(parent) , m_vec(vec), m_counter(0)
  {
    if (!m_vec.size())
      m_vec.push_back(parent);

    m_component = vec[0];
  }

  template <class T2>
  Component_iterator(Component_iterator<T2> const& other)
   :  m_parent(other.m_parent),
      m_component(other.m_component),
      m_vec(other.m_vec),
      m_counter(other.m_counter)
  {
  }

private:
  friend class boost::iterator_core_access;
  template <class> friend class Component_iterator;

  void increment()
  {
    if (++m_counter < m_vec.size())
      m_component = m_vec[m_counter];
    else
      m_component = m_parent;
  }

  template <typename T2>
  bool equal(Component_iterator<T2> const& other) const
  {
    return (m_component == other.m_component);
  }

public:
  boost::shared_ptr<T> dereference() const
  {
    return (m_component);
  }

private:
  boost::shared_ptr<T> m_parent;
  boost::shared_ptr<T> m_component;
  std::vector<boost::shared_ptr<T> > m_vec;
  Uint m_counter;
};


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ComponentIterator_hpp
