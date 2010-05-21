#ifndef CF_Common_Component_hpp
#define CF_Common_Component_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/enable_shared_from_this.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/DynamicObject.hpp"
#include "Common/CPath.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Common/XML.hpp"

namespace CF {
namespace Common {

  template<typename CType>
  class Component_iterator;

////////////////////////////////////////////////////////////////////////////////

  /// Base class for defining CF components
  /// @author Tiago Quintino
  /// @todo remove XMLNode and use RapidXML
  /// @todo add ownership of (sub) components
  /// @todo add dumping of the (sub)tree to a string
  /// @todo add registration into CTree
  class Common_API Component :
      public boost::enable_shared_from_this<Component>,
      public ConfigObject,
      public DynamicObject {

  public: // typedef

    /// typedef of pointers to components
    typedef boost::shared_ptr<Component> Ptr;
    /// type for names of components
    typedef std::string CName;
    /// type of this class contruction provider
    typedef Common::ConcreteProvider < Component, NB_ARGS_1 > PROVIDER;
    /// type of first argument of constructor
    typedef const CName& ARG1;
    /// typedef of the iterator
    typedef Component_iterator<Component> Iterator;

  private: // typedef

    /// type for storing the sub components
    typedef std::map < CName , Component::Ptr > CompStorage_t;

  public: // functions

    /// Get the class name
    static std::string getClassName () { return "Component"; }

    /// Configuration Options
    static void defineConfigOptions ( Common::OptionList& options ) {}

    /// Contructor
    /// @param name of the component
    /// @param parent path where this component will be placed
    Component ( const CName& name );

    /// Virtual destructor
    virtual ~Component();

    /// Get the componment throught the links to the actual components
    virtual Component::Ptr  get ();

    template <typename CType>
        Component_iterator<CType> begin()
    {
      std::vector<boost::shared_ptr<CType> > vec;
      filter_components<CType>(vec);
      return Component_iterator<CType>(vec);
    }

    Component_iterator<Component> end();

    template <typename CType>
    void filter_components(std::vector<boost::shared_ptr<CType> >& vec)
    {
      std::vector<boost::shared_ptr<CType> > components = get_components_by_type<CType>();
      for (typename std::vector<boost::shared_ptr<CType> >::iterator component=components.begin(); component!=components.end(); ++component)
      {
        vec.push_back((*component));
        (*component)->filter_components<CType>(vec);
      }
    }

    /// checks if this component is in fact a link to another component
    bool is_link () const { return m_is_link; }

    /// @return vector of (sub)components with a given type automatically cast to the specified type
    template <typename T>
        std::vector<boost::shared_ptr<T> > get_components_by_type ();

    /// @return vector of (sub)components with a given tag
    std::vector<Component::Ptr> get_components_by_tag(const std::string& tag);

    /// @return vector of (sub)components with a given tag automatically cast to the specified type
    template <typename T>
        std::vector<boost::shared_ptr<T> > get_components_by_tag(const std::string& tag);

    /// Check if this component has a given tag assigned
    bool has_tag(const std::string& tag);

    /// Check if this component has a (sub)components of a given type
    template <typename T>
        bool has_component_of_type();

    /// Check if this component has a (sub)components with a given tag
    bool has_component_with_tag(const std::string& tag);

    /// Access the name of the component
    CName name () const { return m_name.string(); }

    /// Rename the component
    void rename ( const CName& name );

    /// Access the path of the component
    const CPath& path () const { return m_path; }

    /// Modify the parent of this component
    void change_parent ( boost::shared_ptr<Component> new_parent );

    /// Construct the full path
    CPath full_path () const { return m_path / m_name; }

    /// Create a (sub)component of this component automatically cast to the specified type
    template < typename TYPE >
        boost::shared_ptr<TYPE> create_component ( const CName& name );

    /// Add a (sub)component of this component
    void add_component ( Component::Ptr subcomp );

    /// Remove a (sub)component of this component
    Component::Ptr remove_component ( const CName& name );

    /// Move this component to within another one
    /// @param new_parent will be the new parent of this component
    void move_component ( Component::Ptr new_parent );

    /// Get a (sub)component of this component
    /// @param name the component
    Component::Ptr get_component ( const CName& name );

    /// Check if a (sub)component of this component exists
    /// @param name the component
    bool check_component ( const CName& name );

    /// Get a (sub)component of this component automatically cast to the specified type
    /// @param name the component
    template < typename TYPE >
        boost::shared_ptr<TYPE> get_component ( const CName& name );

    /// Return the parent component
    Component::Ptr get_parent() { return m_parent.lock(); }

    /// Looks for a component via its path
    /// (wdeconinck: relative path doesn't work if no root is available)
    /// @param path to the component
    Component::Ptr look_component ( const CPath& path );

    /// Resolves relative elements within a path to complete it.
    /// The path may be relative to this component or absolute.
    /// This is strictly a path operation so the path may not actually point anywhere
    /// @param path to a component
    /// @post path statisfies CPath::is_complete()
    /// @post path statisfies CPath::is_absolute()
    void complete_path ( CPath& path );

    /// add tag to this component
    void add_tag(const std::string& tag);

    /// @return tags in a vector
    std::vector<std::string> get_tags();

    /// @name SIGNALS 
    //@{
   
    /// creates a component from this component
    void create_component ( XmlNode& xml );

    /// lists the sub components and puts them on the xml_tree
    void list_tree ( XmlNode& xml );

    /// lists the options of this component
    void list_options ( XmlNode& xml );

    //@} END SIGNALS 
  
  protected: // functions

    /// Must be called in constructor of each derived class
   template <typename TYPE>
    void build_component(TYPE* meself);

  private: // helper functions

   /// tags this class with the classname
    template <typename TYPE>
        void tag_classname ();

  protected: // data

    /// component name (stored as path to ensure validity)
    CPath m_name;
    /// component current path
    CPath m_path;
    /// list of children
    CompStorage_t m_components;
    /// pointer to the root of this tree
    boost::weak_ptr<Component> m_root;
    /// pointer to the parent component
    boost::weak_ptr<Component> m_parent;
    /// is this a link component
    bool m_is_link;
    /// tags merged as one string e.g. ":Component:CRoot:"
    std::string m_tags;

  }; // Component

////////////////////////////////////////////////////////////////////////////////

  template<class CType>
  class Common_API Component_iterator
          : public boost::iterator_facade<Component_iterator<CType>,
                                          CType,
                                          boost::forward_traversal_tag>
  {
  public:
    typedef boost::shared_ptr<CType> CTypePtr;
    Component_iterator()
    {}

    explicit Component_iterator(std::vector<CTypePtr>& vec)
            : m_vec(vec), m_vecIt(m_vec.begin())
    {
      if (m_vec.size()) {
        m_component = vec[0];
      }
    }

    template <class CType2>
    Component_iterator(Component_iterator<CType2> const& other)
      : m_component(other.m_component) {}

    CTypePtr& get_ptr()
    {
      return m_component;
    }

  private:
    friend class boost::iterator_core_access;
    template <class> friend class Component_iterator;

    void increment()
    {
      m_vecIt++;
      if (m_vecIt != m_vec.end()) {
        m_component = (*m_vecIt);
      }
      else {
        m_component = CTypePtr();
      }
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
    std::vector<CTypePtr> m_vec;
    typename std::vector<CTypePtr>::iterator m_vecIt;
  };

////////////////////////////////////////////////////////////////////////////////

template <typename TYPE>
inline void Component::build_component(TYPE* meself)
{
  addConfigOptionsTo<TYPE>();
  tag_classname<TYPE>();
}

////////////////////////////////////////////////////////////////////////////////

template <typename TYPE>
inline void Component::tag_classname ()
{
  add_tag(TYPE::getClassName());
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
    std::vector<boost::shared_ptr<T> > Component::get_components_by_tag(const std::string& tag)
{
  std::vector<boost::shared_ptr<T> > vec;
  for(CompStorage_t::iterator it=m_components.begin(); it!=m_components.end(); ++it)
  {
    if (it->second->has_tag(tag))
      vec.push_back(boost::dynamic_pointer_cast<T>(it->second));
  }
  return vec;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline std::vector<boost::shared_ptr<T> > Component::get_components_by_type ()
{
  return get_components_by_tag<T>(T::getClassName());
}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE >
inline boost::shared_ptr<TYPE> Component::get_component ( const CName& name )
{
  return boost::dynamic_pointer_cast<TYPE>( get_component(name) );
}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE >
inline boost::shared_ptr<TYPE> Component::create_component ( const CName& name )
{
  boost::shared_ptr<TYPE> new_component ( new TYPE(name), Deleter<TYPE>() );
  add_component(new_component);
  return new_component;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
    bool Component::has_component_of_type()
{
  return has_component_with_tag(T::getClassName());
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Component_hpp
