#ifndef CF_ForAllRegions_hpp
#define CF_ForAllRegions_hpp
#include <boost/mpl/for_each.hpp>

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/SF/Types.hpp"


#include "Common/ComponentPredicates.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
      struct IsGeometryElementType
  {
    bool operator()(const CElements& component)
    {
      return IsElementType<TYPE>()( component.element_type() ) && component.has_tag("GeometryElements") ;
    }
  }; // IsElementRegion

  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
  struct IsFieldElementType
  {
    bool operator()(const CElements& component)
    {
      return IsElementType<TYPE>()( component.element_type() ) && component.has_tag("FieldElements") ;
    }
  }; // IsElementRegion
  
  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
  struct IsComponentElementType
  {
    bool operator()(const CElements& component)
    {
      return IsElementType<TYPE>()( component.element_type() );
    }
  }; // IsElementRegion

template < typename Operation >
struct ForAllRegions
{
  CField& field1;
  CField& field2;

  ForAllRegions( CField& field1_in ) : field1(field1_in), field2(field1_in) {}

  ForAllRegions( CField& field1_in, CField& field2_in ) : field1(field1_in), field2(field2_in) {}

  template < typename EType >
      void operator() ( EType& T )
  {

    BOOST_FOREACH(CElements& elements, recursive_filtered_range_typed<CElements>(field1.support(),IsComponentElementType<EType>()))
    {
      CFinfo << "Elements [" << elements.name() << "] of EType [" << EType::type_name() << "]" << CFendl;

      Operation op ( elements );

      const CArray& coordinates = elements.coordinates();
      const CTable& ctable = elements.connectivity_table();

      // loop on elements
      const Uint elem_count = ctable.size();
      for ( Uint elem = 0; elem != elem_count; ++elem )
      {
        std::vector<RealVector> nodes;
        fill_node_list( std::inserter(nodes, nodes.begin()), coordinates, ctable, elem );
        op.template execute<EType>( elem, nodes );
      }
    }
  }

}; // ForAllRegions

/////////////////////////////////////////////////////////////////////////////////////

template<typename COperation>
class CForAllElements : public Component
{
public: // typedefs
  
  typedef boost::shared_ptr<CForAllElements> Ptr;
  typedef boost::shared_ptr<CForAllElements const> ConstPtr;
  friend struct Looper;

public: // functions
  
  /// Contructor
  /// @param name of the component
  CForAllElements ( const CName& name ) : Component(name) {
    BUILD_COMPONENT;
  }
  
  /// Virtual destructor
  virtual ~CForAllElements() {}
  
  /// Get the class name
  static std::string type_name () { return "CForAllElements"; }
  
  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}
  
  // functions specific to the CField component
    
  void loop(CField& field)
  {
    Looper looper(*this,field);
    boost::mpl::for_each< SF::Types >(looper);
  }

private:

  /// Looper defines a functor taking the type that boost::mpl::for_each
  /// passes. It is the core of the looping mechanism.
  struct Looper
  {
  private:
    CForAllElements& this_class;
    CField& field1;
  public:
    Looper(CForAllElements& this_class_in, CField& field1_in ) : this_class(this_class_in), field1(field1_in) {}
        
    template < typename SFType >
    void operator() ( SFType& T )
    {     
      BOOST_FOREACH(CElements& elements, recursive_filtered_range_typed<CElements>(field1,IsComponentElementType<SFType>()))
      {
        this_class.operation.setup( elements );
        
        // loop on elements. Nothing may be virtual starting from here!
        const Uint elem_count = elements.elements_count();
        for ( Uint elem = 0; elem != elem_count; ++elem )
        {
          this_class.operation.template execute<SFType>( elem );
        }
      }
    }
  }; // Looper      
  
private: // helper functions
  
  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}
  
private:
  
  COperation  operation;
  
};

/////////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF






#endif // CF_ForAllRegions_hpp
