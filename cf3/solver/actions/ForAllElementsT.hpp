// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_ForAllElementsT_hpp
#define cf3_solver_actions_ForAllElementsT_hpp

#include <boost/mpl/for_each.hpp>

#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

#include "mesh/ElementTypes.hpp"
#include "mesh/Region.hpp"

#include "solver/actions/Loop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

/////////////////////////////////////////////////////////////////////////////////////

template<typename ActionT>
class solver_actions_API ForAllElementsT : public Loop
{

  /// Predicate class to test if the region contains a specific element type
  template < typename TYPE >
  struct IsShapeFunction
  {
    bool operator()(const mesh::Elements& component)
    {
      return mesh::IsElementType<TYPE>()( component.element_type() );
    }
  };

  /// Looper defines a functor taking the type that boost::mpl::for_each
  /// passes. It is the core of the looping mechanism.
  struct ElementLooper
  {
    private: // data

      /// Region to loop on
      mesh::Region& region;

      /// Operation to perform
      ActionT& op;

    public: // functions

      /// Constructor
      ElementLooper(ActionT& operation, mesh::Region& region_in )
        : region(region_in) , op(operation)
      {}

      /// Operator
      template < typename SFType >
      void operator() ( SFType& T )
      {
        boost_foreach(mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(region,IsShapeFunction<SFType>()))
        {
          op.set_elements(elements);
          if (op.can_start_loop())
          {
            const Uint nb_elem = elements.size();
            for ( Uint elem = 0; elem != nb_elem; ++elem )
            {
              op.select_loop_idx(elem);
              op.execute();
            }
          }
        }
      }

  }; // ElementLooper

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  ForAllElementsT ( const std::string& name ) :
    Loop(name),
    m_action( create_static_component<ActionT>(ActionT::type_name()) )
  {
    regist_typeinfo(this);
  }

  /// Virtual destructor
  virtual ~ForAllElementsT() {}

  /// Get the class name
  static std::string type_name () { return "ForAllElementsT<" + ActionT::type_name() + ">"; }

  /// const access to the LoopOperation
  virtual const LoopOperation& action(const std::string& name = ActionT::type_name()) const
  {
    return *m_action->template handle<LoopOperation>();
  }

  /// non-const access to the LoopOperation
  virtual LoopOperation& action(const std::string& name = ActionT::type_name())
  {
    return *m_action->template handle<LoopOperation>();
  }

  /// Execute the loop for all elements
  virtual void execute()
  {
    boost_foreach(Handle< mesh::Region >& region, m_loop_regions)
    {
      CFinfo << region->uri().string() << CFendl;

      ElementLooper loop_elements(*m_action,*region);
      boost::mpl::for_each< mesh::ElementTypes >(loop_elements);
    }
  }

private: // data

  /// Operation to perform
  Handle< ActionT > m_action;

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ForAllElementsT_hpp
