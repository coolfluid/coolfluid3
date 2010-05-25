#ifndef CF_Common_ComponentIterator_hpp
#define CF_Common_ComponentIterator_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/iterator/iterator_facade.hpp>

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

template<class CType>
class Component_iterator
        : public boost::iterator_facade<Component_iterator<CType>,
                                        CType,
                                        boost::forward_traversal_tag>
{
public:
  typedef boost::shared_ptr<CType> CTypePtr;

  explicit Component_iterator(const CTypePtr& parent)
    : m_component(parent) , m_parent(parent), m_counter(0)
  {
    m_vec.push_back(m_parent);
  }

  explicit Component_iterator(std::vector<CTypePtr> vec, const CTypePtr& parent)
          : m_component(parent) , m_parent(parent) , m_vec(vec), m_counter(0)
  {
    if (!m_vec.size())
      m_vec.push_back(parent);

    m_component = vec[0];
  }

  template <class CType2>
  Component_iterator(Component_iterator<CType2> const& other)
    : m_component(boost::dynamic_pointer_cast<CType>(other.m_component))
  {
  }

  CTypePtr& get_ptr()
  {
    return m_component;
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

  template <typename CType2>
  bool equal(Component_iterator<CType2> const& other) const
  {
    return (m_component == other.m_component);
  }

public:
  CType& dereference() const
  {
    return *m_component;
  }

private:
  CTypePtr m_component;
  CTypePtr m_parent;
  std::vector<CTypePtr> m_vec;
  Uint m_counter;
};


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ComponentIterator_hpp
