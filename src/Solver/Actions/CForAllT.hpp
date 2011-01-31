// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CForAllT_hpp
#define CF_Solver_Actions_CForAllT_hpp

#include <boost/mpl/for_each.hpp>

#include "Common/Foreach.hpp"

#include "Mesh/SF/Types.hpp"
#include "Mesh/CRegion.hpp"

#include "Solver/Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

/// Predicate class to test for a specific element type
template < typename TYPE >
struct IsElementType
{
  bool operator()(const Mesh::CElements& component)
  {
    return Mesh::IsElementType<TYPE>()( component.element_type() );
  }
}; // IsElementType

/////////////////////////////////////////////////////////////////////////////////////

template< typename ActionT, typename ElemsT >
class Solver_Actions_API CForAllT : public CLoop
{
public: // typedefs

  typedef boost::shared_ptr< CForAllT > Ptr;
  typedef boost::shared_ptr< CForAllT const > ConstPtr;

  friend struct Looper;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllT ( const std::string& name ) : CLoop(name),
    m_action( Common::allocate_component<ActionT>( ActionT::type_name()) )
  {
    regist_typeinfo(this);
    add_static_component ( m_action );
  }

  /// Virtual destructor
  virtual ~CForAllT() {}

  /// Get the class name
  static std::string type_name () { return "CForAllT<" + ActionT::type_name() + ">"; }

  // functions specific to the CForAllElements component

  virtual const CLoopOperation& action(const std::string& name = ActionT::type_name()) const
  {
    return *m_action->as_type<CLoopOperation>();
  }

  virtual CLoopOperation& action(const std::string& name = ActionT::type_name())
  {
    return *m_action->as_type<CLoopOperation>();
  }

  virtual void execute()
  {
//    CFinfo << "execute for all elems" << CFendl;

    BOOST_FOREACH(Mesh::CRegion::Ptr& region, m_loop_regions)
    {
//      CFinfo << "execute for " << region->full_path().path() << CFendl;
//      CFinfo << "This region has " << region->recursive_elements_count() << " elements and ";
//      CFinfo << region->recursive_nodes_count() << " nodes " << CFendl;
      Looper looper(*this,*region);
      boost::mpl::for_each< ElemsT >( looper );
    }
  }

private:

  /// Looper defines a functor taking the type that boost::mpl::for_each
  /// passes. It is the core of the looping mechanism.
  struct Looper
  {
  public: // functions

    /// Constructor
    Looper(CForAllT& this_class, Mesh::CRegion& region_in ) : region(region_in) , op(*this_class.m_action)
    {  /* CFinfo << " building Looper " << CFendl;*/ }

    /// Operator
    template < typename SFType >
    void operator() ( SFType& T )
    {
//      CFinfo << " Looper::operator() " << CFendl;

      boost_foreach(Mesh::CElements& elements, Common::find_components_recursively_with_filter<Mesh::CElements>(region,IsElementType<SFType>()))
      {
//        CFinfo << " .. for elements " << elements.full_path().path() << CFendl;
        op.create_loop_helper( elements );

        // loop on elements. Nothing should be virtual starting from here!
        const Uint elem_count = elements.size();
//        CFinfo << "ELEM COUNT: " << elem_count << CFendl;
        for ( Uint elem = 0; elem != elem_count; ++elem )
        {
          op.select_loop_idx(elem);
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
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CForAllT_hpp
