// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CForAllElementsT2_hpp
#define CF_Solver_Actions_CForAllElementsT2_hpp

#include <boost/mpl/for_each.hpp>

#include "Common/Foreach.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/SF/Types.hpp"
#include "Mesh/CRegion.hpp"

#include "Solver/Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

template<typename ActionT>
class Solver_Actions_API CForAllElementsT2 : public CLoop
{

  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
  struct IsShapeFunction
  {
    bool operator()(const Mesh::CElements& component)
    {
      return Mesh::IsElementType<TYPE>()( component.element_type() );
    }
  };

  /// Looper defines a functor taking the type that boost::mpl::for_each
  /// passes. It is the core of the looping mechanism.
  struct ElementLooper
  {
    private: // data

      /// Region to loop on
      Mesh::CRegion& region;

      /// Operation to perform
      ActionT& op;

    public: // functions

      /// Constructor
      ElementLooper(ActionT& operation, Mesh::CRegion& region_in )
        : region(region_in) , op(operation)
      {}

      /// Operator
      template < typename SFType >
      void operator() ( SFType& T )
      {
        boost_foreach(Mesh::CElements& elements, Common::find_components_recursively_with_filter<Mesh::CElements>(region,IsShapeFunction<SFType>()))
        {
          op.set_elements(elements);
          if (op.can_start_loop())
          {
            const Uint nb_elem = elements.size();
            for ( Uint elem = 0; elem != nb_elem; ++elem )
            {
              op.select_loop_idx(elem);
              op.template executeT<SFType>();
            }
          }
        }
      }

  }; // ElementLooper

public: // typedefs

  typedef boost::shared_ptr< CForAllElementsT2 > Ptr;
  typedef boost::shared_ptr< CForAllElementsT2 const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllElementsT2 ( const std::string& name ) :
    CLoop(name),
    m_action( Common::allocate_component<ActionT>(ActionT::type_name()) )
  {
    regist_typeinfo(this);
    add_static_component ( m_action );
  }

  /// Virtual destructor
  virtual ~CForAllElementsT2() {}

  /// Get the class name
  static std::string type_name () { return "CForAllElementsT2<" + ActionT::type_name() + ">"; }

  /// const access to the LoopOperation
  virtual const CLoopOperation& action(const std::string& name = ActionT::type_name()) const
  {
    return *m_action->as_ptr<CLoopOperation>();
  }

  /// non-const access to the LoopOperation
  virtual CLoopOperation& action(const std::string& name = ActionT::type_name())
  {
    return *m_action->as_ptr<CLoopOperation>();
  }

  /// Execute the loop for all elements
  virtual void execute()
  {
    boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
    {
      CFinfo << region->full_path().string() << CFendl;
      
      ElementLooper loop_elements(*m_action,*region);
      boost::mpl::for_each< Mesh::SF::Types >(loop_elements);
    }
  }

private: // data

  /// Operation to perform
  typename ActionT::Ptr m_action;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CForAllElementsT2_hpp
