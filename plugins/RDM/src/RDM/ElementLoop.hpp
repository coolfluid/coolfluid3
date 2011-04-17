// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CellLoop_hpp
#define CF_RDM_CellLoop_hpp

#include <boost/mpl/for_each.hpp>

#include "Common/FindComponents.hpp"

#include "Mesh/CRegion.hpp"

#include "RDM/SupportedTypes.hpp"
#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

//------------------------------------------------------------------------------------------

/// Abstract RDM looping component
class RDM_API ElementLoop : public Common::Component {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< ElementLoop > Ptr;
  typedef boost::shared_ptr< ElementLoop const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  ElementLoop ( const std::string& name ) : Common::Component(name) {}

  /// Virtual destructor
  virtual ~ElementLoop() {}

  /// Get the class name
  static std::string type_name () { return "ElementLoop"; }

  /// execute the action
  virtual void execute () = 0;

  /// selects the region where to loop on
  void select_region( Mesh::CRegion::Ptr region ) { current_region = region; }

  /// Access the scheme.
  /// Will create it if does not exist.
  /// @return reference to the scheme
  template < typename SchemeT > SchemeT& access_scheme()
  {
    Common::Component::Ptr cscheme = parent()->get_child_ptr( SchemeT::type_name() );
    typename SchemeT::Ptr scheme;
    if( is_null( cscheme ) )
      scheme = parent()->template create_component< SchemeT >( SchemeT::type_name() );
    else
      scheme = cscheme->as_ptr_checked<SchemeT>();
    return *scheme;
  }

protected: // data

  /// region to loop on
  Mesh::CRegion::Ptr current_region;

}; // ElementLoop

//------------------------------------------------------------------------------------------

/// CellLoop defines a functor taking the type that boost::mpl::for_each passes.
/// It is the core of the looping mechanism over Cells.
template < typename SCHEME, typename PHYS>
struct CellLoop : public ElementLoop
{
  /// Constructor
  CellLoop( const std::string& name ) : ElementLoop(name) {  regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "CellLoop<" + SCHEME::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ()
  {
    boost::mpl::for_each< typename RDM::ElemTypes< PHYS::ndim >::Cells >( boost::ref(*this) );
  }

  /// operator needed for the loop over element types (SF)
  template < typename SF >
  void operator() ( SF& )
  {
    if( is_null(parent()->as_ptr<SCHEME>()) )
      throw Common::SetupError(FromHere(), type_name() + " was intantiated with wrong scheme");

    /// definition of the quadrature type
    typedef typename RDM::DefaultQuadrature<SF>::type QD;
    /// parametrization of the numerical scheme
    typedef typename SCHEME::template Scheme< SF, QD, PHYS > SchemeT;

    SchemeT& scheme = this->access_scheme<SchemeT>();

    // loop on the (sub)regions that hold elements of this type

    boost_foreach(Mesh::CElements& elements,
                  Common::find_components_recursively_with_filter<Mesh::CElements>(*current_region,IsElementType<SF>()))
    {
      // point the scheme to the elements of the (sub)region
      scheme.set_elements(elements);

      const Uint nb_elem = elements.size();
      for ( Uint elem = 0; elem != nb_elem; ++elem )
      {
        scheme.select_loop_idx(elem);
        scheme.execute();
      }
    }
  }

}; // CellLoop

//------------------------------------------------------------------------------------------

/// FaceLoop defines a functor taking the type that boost::mpl::for_each passes.
/// It is the core of the looping mechanism over Faces.
template < typename SCHEME, typename PHYS>
struct FaceLoop : public ElementLoop
{
  /// Constructor
  FaceLoop( const std::string& name ) : ElementLoop(name) {  regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "FaceLoop<" + SCHEME::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ()
  {
    boost::mpl::for_each< typename RDM::ElemTypes< PHYS::ndim >::Faces >( boost::ref(*this) );
  }

  /// operator needed for the loop over element types (SF)
  template < typename SF >
  void operator() ( SF& )
  {
    if( is_null(parent()->as_ptr<SCHEME>()) )
      throw Common::SetupError(FromHere(), type_name() + " was intantiated with wrong scheme");

    /// definition of the quadrature type
    typedef typename RDM::DefaultQuadrature<SF>::type QD;
    /// parametrization of the numerical scheme
    typedef typename SCHEME::template Scheme< SF, QD, PHYS > SchemeT;

    SchemeT& scheme = this->access_scheme<SchemeT>();

    // loop on the (sub)regions that hold elements of this type

    boost_foreach(Mesh::CElements& elements,
                  Common::find_components_recursively_with_filter<Mesh::CElements>(*current_region,IsElementType<SF>()))
    {
      // point the scheme to the elements of the (sub)region
      scheme.set_elements(elements);

      const Uint nb_elem = elements.size();
      for ( Uint elem = 0; elem != nb_elem; ++elem )
      {
        scheme.select_loop_idx(elem);
        scheme.execute();
      }
    }
  }

}; // FaceLoop

//------------------------------------------------------------------------------------------

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_CellLoop_hpp
