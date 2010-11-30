// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CForAllElementsT_hpp
#define CF_Mesh_CForAllElementsT_hpp

#include "Mesh/SF/Types.hpp"
#include "Mesh/CRegion.hpp"

#include "Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

/// Predicate class to test if the region contains a specific element type
template < typename TYPE >
struct IsComponentElementType
{
  bool operator()(const Mesh::CElements& component)
  {
    return Mesh::IsElementType<TYPE>()( component.element_type() );
  }
}; // IsElementRegion

/////////////////////////////////////////////////////////////////////////////////////

template<typename ActionT>
class Actions_API CForAllElementsT : public CLoop
{
public: // typedefs

  typedef boost::shared_ptr< CForAllElementsT > Ptr;
  typedef boost::shared_ptr< CForAllElementsT const > ConstPtr;
  friend struct Looper;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllElementsT ( const std::string& name ) :
    CLoop(name),
    m_action( Common::allocate_component_type<ActionT>(ActionT::type_name()) )
  {
     

    add_static_component ( m_action );
  }

  /// Virtual destructor
  virtual ~CForAllElementsT() {}

  /// Get the class name
  static std::string type_name () { return "CForAllElements"; }

  // functions specific to the CForAllElements component

  virtual const CLoopOperation& action(const std::string& name = ActionT::type_name()) const
  {
    return *m_action->get_type<CLoopOperation>();
  }

  virtual CLoopOperation& action(const std::string& name = ActionT::type_name())
  {
    return *m_action->get_type<CLoopOperation>();
  }
  
  virtual void execute()
  {
    BOOST_FOREACH(Mesh::CRegion::Ptr& region, m_loop_regions)
    {
      Looper looper(*this,*region);
      boost::mpl::for_each< Mesh::SF::Types >(looper);
    }
  }

private:

  /// Looper defines a functor taking the type that boost::mpl::for_each
  /// passes. It is the core of the looping mechanism.
  struct Looper
  {
  public: // functions

    /// Constructor
    Looper(CForAllElementsT& this_class, Mesh::CRegion& region_in ) : region(region_in) , op(*this_class.m_action) { }

    /// Operator
    template < typename SFType >
    void operator() ( SFType& T )
    {
      BOOST_FOREACH(Mesh::CElements& elements, Common::recursive_filtered_range_typed<Mesh::CElements>(region,IsComponentElementType<SFType>()))
      {
        op.set_loophelper( elements );

        // loop on elements. Nothing may be virtual starting from here!
        const Uint elem_count = elements.elements_count();
        for ( Uint elem = 0; elem != elem_count; ++elem )
        {
          op.set_loop_idx(elem);
          op.template executeT<SFType>( );
        }
      }
    }

  private: // data

    /// Region to loop on
    Mesh::CRegion& region;

    /// Operation to perform
    ActionT& op;

  }; // Looper

private: // data

  /// Operation to perform
  typename ActionT::Ptr m_action;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CForAllElementsT_hpp
