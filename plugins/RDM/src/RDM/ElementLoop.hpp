// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_ElementLoop_hpp
#define CF_RDM_ElementLoop_hpp

#include <boost/mpl/for_each.hpp>

#include "Common/FindComponents.hpp"

#include "Mesh/CRegion.hpp"

#include "RDM/SupportedTypes.hpp"
#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API DomainLoop : public Common::Component {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< DomainLoop > Ptr;
  typedef boost::shared_ptr< DomainLoop const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  DomainLoop ( const std::string& name ) : Common::Component(name) {}

  /// Virtual destructor
  virtual ~DomainLoop() {}

  /// Get the class name
  static std::string type_name () { return "DomainLoop"; }

  /// execute the action
  virtual void execute () = 0;

  void select_region( Mesh::CRegion::Ptr region ) { current_region = region; }

  /// region to loop on
  Mesh::CRegion::Ptr current_region;

}; // DomainLoop

/// Looper defines a functor taking the type that boost::mpl::for_each passes.
/// It is the core of the looping mechanism.
template < typename SCHEME, typename PHYS>
struct ElementLoop : public DomainLoop
{
  /// Constructor
  ElementLoop( const std::string& name ) : DomainLoop(name) {  regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "ElementLoop<" + SCHEME::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ()
  {
    boost::mpl::for_each< RDM::CellTypes >( boost::ref(*this) );
  }

  /// operator needed for the loop over element types (SF)
  template < typename SF >
  void operator() ( SF& T )
  {
    typename SCHEME::Ptr comp = parent()->as_ptr_checked<SCHEME>();

    /// definition of the quadrature type
    typedef typename RDM::DefaultQuadrature<SF>::type QD;
    /// parametrization of the numerical scheme
    typedef typename SCHEME::template Scheme< SF, QD, PHYS > SchemeT;

    boost_foreach(Mesh::CElements& elements,
                  Common::find_components_recursively_with_filter<Mesh::CElements>(*current_region,IsElementType<SF>()))
    {
      // get the scheme or create it if does not exist
      Common::Component::Ptr cscheme = comp->get_child_ptr( SchemeT::type_name() );
      typename SchemeT::Ptr scheme;
      if( is_null( cscheme ) )
        scheme = comp->template create_component< SchemeT >( SchemeT::type_name() );
      else
        scheme = cscheme->as_ptr_checked<SchemeT>();

      // loop on elements of that type
      scheme->set_elements(elements);

      const Uint nb_elem = elements.size();
      for ( Uint elem = 0; elem != nb_elem; ++elem )
      {
        scheme->select_loop_idx(elem);
        scheme->execute();
      }
    }
  }

}; // ElementLoop

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_ElementLoop_hpp
