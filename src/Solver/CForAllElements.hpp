#ifndef CF_Mesh_CForAllElements_hpp
#define CF_Mesh_CForAllElements_hpp

#include <boost/mpl/for_each.hpp>

#include "Mesh/COperation.hpp"
#include "Mesh/SF/Types.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

/////////////////////////////////////////////////////////////////////////////////////

/// Predicate class to test if the region contains a specific element type
template < typename TYPE >
struct IsComponentElementType
{
  bool operator()(const CElements& component)
  {
    return IsElementType<TYPE>()( component.element_type() );
  }
}; // IsElementRegion

/////////////////////////////////////////////////////////////////////////////////////

template<typename COp>
class Mesh_API CForAllElementsT : public COperation
{
public: // typedefs

  typedef boost::shared_ptr< CForAllElementsT > Ptr;
  typedef boost::shared_ptr< CForAllElementsT const > ConstPtr;
  friend struct Looper;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllElementsT ( const CName& name ) :
    COperation(name), 
    m_operation(new COp("operation"), Deleter<COp>()) 
  {
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~CForAllElementsT() {}

  /// Get the class name
  static std::string type_name () { return "CForAllElements"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  // functions specific to the CForAllElements component

  void needs_regions(std::vector<CRegion::Ptr>& regions)
  {
    m_loop_regions = regions;
  }
  
  const COp& operation() const
  {
    return *m_operation;
  }
  
  COp& operation()
  {
    return *m_operation;
  }
  
  void execute(Uint index = 0)
  {
    // If the typename of the operation equals "COperation", then the virtual version
    // must have been called. In this case the operation must have been created as a 
    // child component of "this_class", and should be set accordingly.
    if (m_operation->type_name() == "COperation")
    {
      BOOST_FOREACH(CRegion::Ptr& region, m_loop_regions)
        BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(*region))
      {
        // Setup all child operations
        BOOST_FOREACH(COperation& operation, range_typed<COperation>(*this))
          operation.setup( elements );

        // loop on elements. Things will still be virtual starting from here!
        const Uint elem_count = elements.elements_count();
        for ( Uint elem = 0; elem != elem_count; ++elem )
        {
          // Execute all child operations on this element
          BOOST_FOREACH(COperation& operation, range_typed<COperation>(*this))
            operation.execute( elem );
        }
      }
    }
    else
    // Use now the templated version defined below
    {
      BOOST_FOREACH(CRegion::Ptr& region, m_loop_regions)
      {
        Looper looper(*this,*region);
        boost::mpl::for_each< SF::Types >(looper);
      }      
    }
  }

private:

  /// Looper defines a functor taking the type that boost::mpl::for_each
  /// passes. It is the core of the looping mechanism.
  struct Looper
  {
  private:
    CForAllElementsT& this_class;
    CRegion& region;
  public:
    Looper(CForAllElementsT& this_class_in, CRegion& region_in ) : this_class(this_class_in), region(region_in) 
    {
    }

    template < typename SFType >
    void operator() ( SFType& T )
    { 
      BOOST_FOREACH(CElements& elements, recursive_filtered_range_typed<CElements>(region,IsComponentElementType<SFType>()))
      {
        this_class.m_operation->setup( elements );

        // loop on elements. Nothing may be virtual starting from here!
        const Uint elem_count = elements.elements_count();
        for ( Uint elem = 0; elem != elem_count; ++elem )
        {
          this_class.m_operation->template executeT<SFType>( elem );
        }
      }
    }
  }; // Looper      

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private:

  typename COp::Ptr m_operation;

  std::vector<CRegion::Ptr> m_loop_regions;

};

typedef CForAllElementsT<COperation> CForAllElements;

/////////////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_COperation_hpp
